/******************************************************************************

 @File        gl_sterio3d.cpp 

 @Title       OpenGL ES 1.x Based Anaglyph Example

 @Version     1.0 

 @Platform    Linux

 @Description  Basic example that shows how 3D-Anaglyph works with Opengl ES1.x

******************************************************************************/
#include <stdio.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <math.h>

/******************************************************************************
 Defines
******************************************************************************/
//helper class for stereo visualisation using OpenGL

class StereoClass {
public:
	StereoClass();
	void Update();

/* Headerfile gathering info from StereoClass.cpp 
for use within Source Code.cpp */

public:
	float w,h,z,a,Near,Far;
	float L_l, L_r, L_b ,L_t;
	float R_l ,R_r ,R_b ,R_t;

	GLfloat LookAtLeft[9]; // Lookat Left 9 arrays 
	GLfloat LookAtRight[9]; // Lookat Left 9 arrays 
	GLfloat FrustumLeft[6]; // FrustumLeft 6 arrays 
	GLfloat FrustumRight[6]; // FrustumLeft 6 arrays 
};

GLuint texture;
// Interleaved vertex data
float afVertices[] = {	-0.4f,-0.4f,0.0f, // Position
			1.0f,1.0f,0.66f,1.0f, // Color
			0.4f,-0.4f,0.0f,
			1.0f,1.0f,0.66f,1.0f,
			0.0f,0.4f,0.0f,
			1.0f,1.0f,0.66f,1.0f};


#define FIXED_ONE 0x10000
const double PIXELS_PER_INCH = 100.0;
#define M_PI       3.14159265358979323846

int type = 0;

//Animation
float XRotate = 0;

//Initialising the Colours
GLfloat Red[] = {1.0f, 0.0f, 0.0f, 1.0f}; // variable for colour Red
GLfloat Green[] = {0.0f, 1.0f, 0.0f, 1.0f}; // variable for colour Green
GLfloat White[] = {1.0f, 1.0f, 1.0f, 1.0f}; // variable for colour White

//Type of 3D false so 'None' will be shown at start (void Display)
bool Parallel = true; 

//Initialising arrays. f has 6 values, l has 9 and p has 4.
GLfloat f[6]; //frustum
GLfloat l[9]; //look at
GLfloat p[4]; //perspective

//StereoCamera initialisation
StereoClass sterioObj;

// EGL variables
EGLDisplay			eglDisplay	= 0;
EGLConfig			eglConfig	= 0;
EGLSurface			eglSurface	= 0;
EGLContext			eglContext	= 0;
	
GLuint	ui32Vbo = 0; // Vertex buffer object handle


void render_static();


// C++ class for stereo visualisation using OpenGL
// The underlying equations and their implementation are by courtesy of 
// Peter Hughes, Durham University 2009.
//Constructor
StereoClass::StereoClass() {
	
}

void StereoClass::Update() {
#if 1
	w = 100.0f; // Physical display dimension in mm (width)
	h = 62.5f; // height
	a = 12.54f; // Camera inter-axial seperation (eye seperation).
#endif
	z = 1000.0; // Distance in the scene from the camera to the display plane.
	a = 1.5f; // Camera inter-axial seperation (eye seperation).
	Near = 100.0; // Distance in the scene from the camera to the near plane.
	Far = 1600.0f; //Distance in the scene from the camera to the far plane.

	//Calculations for Left eye/camera frustum
	L_l = -( Near * ( ( w/2 - a/2) / z) ); // Left clipping pane
	L_r = ( Near * ( ( w/2.0 + a/2.0) / z) ); // Right clipping pane
	L_b = -( Near * ( ( h/2.0) / z) ); // Bottom clipping pane
	L_t = ( Near * ( ( h/2.0) / z) ); // Top clipping pane

	//Calculations for Right eye/camera frustum
	R_l = -( Near * ( ( w/2.0 + a/2.0) / z) ); // Left clipping pane
	R_r = ( Near * ( ( w/2.0 - a/2.0) / z) ); // Right clipping pane
	R_b = -( Near * ( ( h/2.0) / z) ); // Bottom clipping pane
	R_t = ( Near * ( ( h/2.0) / z) );// Top clipping pane

	// Lookat points for left eye/camera
	LookAtLeft[0] = (-a/2);
	LookAtLeft[1] = 0.0f;
	LookAtLeft[2] = 0.0f;
	LookAtLeft[3] = (-a/2);
	LookAtLeft[4] = 0.0f;
	LookAtLeft[5] = -z;
	LookAtLeft[6] = 0.0f;
	LookAtLeft[7] = 1.0f;
	LookAtLeft[8] = 0.0f;

	// Lookat points for right eye/camera
	LookAtRight[0] = (a/2);
	LookAtRight[1] = 0.0f;
	LookAtRight[2] = 0.0f;
	LookAtRight[3] = (a/2);
	LookAtRight[4] = 0.0f;
	LookAtRight[5] = -z;
	LookAtRight[6] = 0.0f;
	LookAtRight[7] = 1.0f;
	LookAtRight[8] = 0.0f;

	// Parameters for glFrustum (Left)
	FrustumLeft[0] = L_l;
	FrustumLeft[1] = L_r;
	FrustumLeft[2] = L_b;
	FrustumLeft[3] = L_t;
	FrustumLeft[4] = Near;
	FrustumLeft[5] = Far;

	// Parameters for glFrustums (Right)
	FrustumRight[0] = R_l;
	FrustumRight[1] = R_r;
	FrustumRight[2] = R_b;
	FrustumRight[3] = R_t;
	FrustumRight[4] = Near;
	FrustumRight[5] = Far;
}


/*!****************************************************************************
 @Function		TestEGLError
 @Input			pszLocation		location in the program where the error took
								place. ie: function name
 @Return		bool			true if no EGL error was detected
 @Description	Tests for an EGL error and prints it
******************************************************************************/
bool TestEGLError(const char* pszLocation)
{
	/*
		eglGetError returns the last error that has happened using egl,
		not the status of the last called function. The user has to
		check after every single egl call or at least once every frame.
	*/
	EGLint iErr = eglGetError();
	if (iErr != EGL_SUCCESS)
	{
		printf("%s failed (%d).\n", pszLocation, iErr);
		return false;
	}

	return true;
}



static void gluLookAt(float eyeX, float eyeY, float eyeZ,
        float centerX, float centerY, float centerZ, float upX, float upY,
        float upZ)
{
    // See the OpenGL GLUT documentation for gluLookAt for a description
    // of the algorithm. We implement it in a straightforward way:

    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;

    // Normalize f
    float rlf = 1.0f / sqrtf(fx*fx + fy*fy + fz*fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    // Normalize up
    float rlup = 1.0f / sqrtf(upX*upX + upY*upY + upZ*upZ);
    upX *= rlup;
    upY *= rlup;
    upZ *= rlup;

    // compute s = f x up (x means "cross product")

    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;

    // compute u = s x f
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    float m[16] ;
    m[0] = sx;
    m[1] = ux;
    m[2] = -fx;
    m[3] = 0.0f;

    m[4] = sy;
    m[5] = uy;
    m[6] = -fy;
    m[7] = 0.0f;

    m[8] = sz;
    m[9] = uz;
    m[10] = -fz;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;

    glMultMatrixf(m);
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}


// Renders scene in either GLU_BACK_LEFT or GLU_BACK_RIGHT buffer
void Render (int side)
{


	if (side == 1) 
	{ 					
		for (int i = 0; i < 6; i++) 
		{
			f[i] = sterioObj.FrustumLeft[i];
		}

		for (int i = 0; i < 9; i++)
		{
			l[i] = sterioObj.LookAtLeft[i];
		}
	} 

	else 
	{ 
		for (int i = 0; i < 6; i++)
		{
			f[i] = sterioObj.FrustumRight[i];
		}

		for (int i = 0; i < 9; i++)
		{
			l[i] = sterioObj.LookAtRight[i];
		}
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glFrustumf(f[0],f[1],f[2],f[3],f[4],f[5]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(l[0], l[1], l[2], l[3], l[4], l[5], l[6], l[7], l[8]); 

	// draw the array of cubes at varying positions across the screen
	// rotating centre cube
	glTranslatef (0.0f, 0.0f, -1500.0f); // x, y, z

	glPushMatrix();	
	glTranslatef (-60.0f, 0.0f, 0.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();

	glPushMatrix();	
	glTranslatef (-60.0f, -30.0f, 0.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();

	glPushMatrix();	
	glTranslatef (60.0f, 0.0f, 0.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();
	
	glPushMatrix();	
	glTranslatef (60.0f, 30.0f, 0.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();
	
	glPushMatrix();	
	glTranslatef (0.0f, -30.0f, 0.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();

	glPushMatrix();	
	glTranslatef (0.0f, 30.0f, 0.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();
	
	glPushMatrix();	
	glTranslatef (-60.0f, 30.0f, 0.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();
	
	glPushMatrix();	
	glTranslatef (60.0f, -30.0f, 0.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();

	glPushMatrix();	
	glTranslatef (0.0f, 0.0f, 1000.0f); // x, y, z
	glRotatef(XRotate,0.1, 0.1, 0.8);
	render_static();
	glPopMatrix();
	
}

// Set light colour to 'white', 'red' and 'green'
// if parallel and toedin are off, display white
void Light(int Side)
{

	if (Side == 1) 
	{

		glLightfv( GL_LIGHT0, GL_AMBIENT, Red );
		glLightfv( GL_LIGHT0, GL_DIFFUSE, Red );
		glLightfv( GL_LIGHT0, GL_SPECULAR, Red );
	}

	else // displays green if its not side == 1 
	{
		glLightfv( GL_LIGHT0, GL_AMBIENT, Green );
		glLightfv( GL_LIGHT0, GL_DIFFUSE, Green );
		glLightfv( GL_LIGHT0, GL_SPECULAR, Green );
	}
}


void render_static()
{
	GLfloat set=0.0;
	GLuint base_index[] = {  
	                			    1, 3, 2,
	  			                    0, 1, 2
				};



	const GLfloat front[]={				-5.0f, -5.0f, 5.0f + set,
							5.0f, -5.0f, 5.0f + set,
							-5.0f,  5.0f, 5.0f + set,
							5.0f,  5.0f, 5.0f + set
						};

	const GLfloat back[]={				-5.0f, -5.0f, -5.0f - set,
							5.0f, -5.0f, -5.0f - set,
							-5.0f,  5.0f, -5.0f - set,
							5.0f,  5.0f, -5.0f - set
						};

			// LEFT
	const GLfloat left[] ={
							-5.0f, -5.0f,  5.0f + set,
							-5.0f, -5.0f, -5.0f - set,
							-5.0f,  5.0f,  5.0f + set,
							-5.0f,  5.0f, -5.0f - set
						};
			// RIGHT
	const GLfloat right[] = {
							5.0f, -5.0f, 5.0f + set,
							5.0f, -5.0f, -5.0f - set,
							5.0f,  5.0f,  5.0f + set,
							5.0f,  5.0f, -5.0f - set
						};
			// TOP
	const GLfloat top[] = {
							-5.0f, 5.0f, 5.0f + set,
							5.0f, 5.0f, 5.0f + set,
							-5.0f, 5.0f, -5.0f - set,
							5.0f, 5.0f, -5.0f - set
						};	

	const GLfloat bottom[]={
							-5.0f, -5.0f, 5.0f + set,
							5.0f, -5.0f, 5.0f + set,
							-5.0f, -5.0f, -5.0f - set,
							5.0f, -5.0f, -5.0f - set
						};


	
	glDisable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, front);
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, base_index);

	glVertexPointer(3, GL_FLOAT, 0, back);
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, base_index);

	glVertexPointer(3, GL_FLOAT, 0, left);
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, base_index);
	
	glVertexPointer(3, GL_FLOAT, 0, right);
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, base_index);
	
	glVertexPointer(3, GL_FLOAT, 0, top);
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, base_index);

	glVertexPointer(3, GL_FLOAT, 0, bottom);
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, base_index);

}

void draw_triangle()
{

	/*
	Draw a triangle
	*/

	// Enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);

	/*
	   Set the vertex pointer.
	 */
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 7, 0);

	// Set color data in the same way
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4,GL_FLOAT,sizeof(float) * 7, (GLvoid*) (sizeof(float) * 3) /*The color starts after the 3 position values (x,y,z)*/);

	/*
		Draws a non-indexed triangle array from the pointers previously given.
	*/
	glDrawArrays(GL_TRIANGLES, 0, 3);
	if (!TestEGLError("glDrawArrays"))
	{
		goto cleanup;
	}

cleanup:
 	return;
}


// OpenGL Functions setting light, colour, texture etc
void Init()

{	
	//Clear screen of all colours
	glClearColor(0,0,0,0);

	//Depth Test
	glEnable(GL_DEPTH_TEST);

	//Shade model
	glShadeModel(GL_SMOOTH);

	//Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0); 
	glLightModelx(GL_LIGHT_MODEL_TWO_SIDE, 0);

	//Specify different types of light
	GLfloat Light0Pos[] = {4.0f, 4.0f, 4.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, Light0Pos );
	
	GLfloat Light0Amb[] = {0.8f, 0.8f, 0.8f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, Light0Amb ); 

	GLfloat Light0Diff[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Light0Diff ); 

	GLfloat Light0Spec[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_SPECULAR, Light0Spec ); 

	//Set Materials
	GLfloat MaterialAmb[] = {0.2f, 0.2f, 0.2f, 1.0f};
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT , MaterialAmb);

	GLfloat MaterialDif[] = {0.7f, 0.7f, 0.7f, 1.0f};
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE , MaterialDif);

	GLfloat MaterialSpec[] = {0.5f, 0.5f, 0.5f, 1.0f};
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR , MaterialSpec);

	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS , 50.0f);

	sterioObj.Update();
}

void initEGL()
{
	/*
		Step 1 - Get the default display.
	*/
	eglDisplay = eglGetDisplay((NativeDisplayType)0);

	/*
		Step 2 - Initialize EGL.
	*/
	EGLint iMajorVersion, iMinorVersion;
	if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
	{
		printf("Error: eglInitialize() failed.\n");
		goto cleanup;
	}

	/*
		Step 3 - Specify the required configuration attributes.
	 */
	EGLint pi32ConfigAttribs[3];
	pi32ConfigAttribs[0] = EGL_SURFACE_TYPE;
	pi32ConfigAttribs[1] = EGL_WINDOW_BIT;
	pi32ConfigAttribs[2] = EGL_NONE;

	/*
		Step 4 - Find a config that matches all requirements.
	*/
	EGLint iConfigs;
	if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
	{
		printf("Error: eglChooseConfig() failed.\n");
		goto cleanup;
	}

	/*
		Step 5 - Create a surface to draw to.
	*/
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)0, NULL);
	if (!TestEGLError("eglCreateWindowSurface"))
	{
			goto cleanup;
	}

	/*
		Step 6 - Create a context.
	*/
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
	if (!TestEGLError("eglCreateContext"))
	{
		goto cleanup;
	}

	/*
		Step 7 - Bind the context to the current thread and use our
	*/
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (!TestEGLError("eglMakeCurrent"))
	{
		goto cleanup;
	}
cleanup:

	return;

}

/*!****************************************************************************
 @Function		main
 @Return		int			result code to OS
 @Description	Main function of the program
******************************************************************************/
int main()
{
    initEGL();
    Init();
    //create_texture();

	for(;;)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		/*
			Clears the color buffer.
			glClear() can also be used to clear the depth or stencil buffer
			(GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)
		*/
		glClearColor(0.0, 0.5, 0.5, 1.0);
		glClear(GL_DEPTH_BUFFER_BIT);
		if (!TestEGLError("glClear"))
		{
			goto cleanup;
		}


		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(false, true, true, true);
		Light(0); 
		Render(0); 
		glClear( GL_DEPTH_BUFFER_BIT) ;
		glColorMask(true, false, false, true);
		Light(1); 
		Render(1); 
		glColorMask( true, true, true, true);

		XRotate+=0.5; 
		/*
			Swap Buffers.
			Brings to the native display the current render surface.
		*/
		eglSwapBuffers(eglDisplay, eglSurface);
		if (!TestEGLError("eglSwapBuffers"))
		{
			goto cleanup;
		}
	}

	/*
		Step 8 - Terminate OpenGL ES and destroy the window (if present).
		eglTerminate takes care of destroying any context or surface created
	*/
cleanup:
	// Delete the VBO as it is no longer needed
	glDeleteBuffers(1, &ui32Vbo);
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ;
	eglTerminate(eglDisplay);

	return 0;
}

