//
//  main.c
//  Fractal Generator and Navigator
//
//  Created by Juan Miguel Rubio on 04/03/14.
//  Copyright (c) 2014 Juan Miguel Rubio. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <complex.h>
#include <string.h>

#include "threads_helper.h"

#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
#else
#ifdef _WIN32
    #include <windows.h>
#endif
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
#endif

#define WIN_DISP_W  800
#define WIN_DISP_H  800
#define WIN_INFO_W  220
#define WIN_INFO_H  800

typedef struct {unsigned char r, g, b;} rgb_t;
rgb_t **tex = 0;
int gwin_display, gwin_info;
GLuint texture;
int width, height;
int tex_w, tex_h;

double scale = 1./256;
double cx = 0, cy = 0;
int color_rotate = 0;
int saturation = 1;
int invert = 0;
int mandel_min, mandel_max;

int julia = 0;
// function to evaluate
int fz = 1;
// constant of the function to evaluate
double vr = 0.0;
double vi = 0.0;

int max_iter = 128;
int target_max_iter = 128;

int zoom = 0;
double vrpivot = 0;
double vipivot = 0;

double target_cx = 0.0;
double target_cy = 0.0;

double target_scale = 1./256;
double target_vr = 0;
double target_vi = 0;

int lock_vr = 0;
int lock_vi = 0;

int vprecision = 8;

// glut helper vars
int valuemouse = 0;
int mouseX = 0;
int mouseY = 0;
double animation_stop = 1;
int render_num = 0;

unsigned int iterations[WIN_DISP_W][WIN_DISP_H];

// threads helper vars
int max_threads = 0;

void set_texture ();


void printtext (int x, int y, const char *str)
{
	int len = (int) strlen(str);
	unsigned int i;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, WIN_DISP_W, 0, WIN_INFO_H, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glRasterPos2i(x, y);
	for (i = 0; (unsigned) i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
	}
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void display ()
{
    char lines;
    int i;
    int pos_y;
    
	char str1[32];
    char str2[32];
    char str3[32];
    char str4[32];
    char str5[32];
    char str6[32];
    char str7[32];
    
	snprintf(str1, sizeof(str1), "        vr: %f", vr);
    snprintf(str2, sizeof(str2), "        vi: %f", vi);
    snprintf(str3, sizeof(str3), "        cx: %f", cx);
    snprintf(str4, sizeof(str4), "        cy: %f", cy);
    snprintf(str5, sizeof(str5), "     scale: %f", scale);
    snprintf(str6, sizeof(str6), "iterations: %d", max_iter);
    
	glClear(GL_COLOR_BUFFER_BIT);
	glPushAttrib(GL_CURRENT_BIT);
    glColor3f(1, 0, 0);
    printtext(0, WIN_INFO_H - 15, "Values:");
	glColor3f(1, 1, 1);
	printtext(10, WIN_INFO_H - 30, str1);
    printtext(10, WIN_INFO_H - 45, str2);
    printtext(10, WIN_INFO_H - 60, str3);
    printtext(10, WIN_INFO_H - 75, str4);
    printtext(10, WIN_INFO_H - 90, str5);
    printtext(10, WIN_INFO_H - 105, str6);
    
    glColor3f(0, 0, 1);
    snprintf(str7, sizeof(str7), "    render: %d", render_num);
    printtext(10, WIN_INFO_H - 120, str7);
    
    /* Print the key controls */
    glColor3f(1, 0, 0);
    printtext(0, WIN_INFO_H - 140, "Keys:");
    
    lines = 30;
    const char *str_keys[30] = {
        "  esc: reset",
        "",
        "    1: mandelbrot set",
        "    2: julia set z2",
        "    3: julia set z2",
        "    4: julia set z2",
        "    5: julia set z3",
        "",
        "space: invert colors",
        "    r: color rotation",
        "    c: monochrome",
        "",
        "    a: move right",
        "    s: move up",
        "    d: move left",
        "    w: move down",
        "",
        "    q: zoom out",
        "    e: zoom in",
        "",
        "    f: decrease real C",
        "    g: decrease imag C",
        "    h: increase real C",
        "    t: increase imag C",
        "",
        "    z: dec iterations",
        "    x: inc iterations",
        "",
        "    o: export to CSV",
        "    p: export to PPM"
    };
    
    glColor3f(1, 1, 1);
    for(i = lines - 1; i >= 0; i--){
        pos_y = WIN_INFO_H - 155 - i * 15;
        printtext(10, pos_y, str_keys[i]);
    }
    
    /* Print the mouse controls */
    glColor3f(1, 0, 0);
    printtext(0, WIN_INFO_H - 625, "Mouse:");
    
    lines = 8;
    const char *str_mouse[8] = {
        "left click:",
        "    zoom in",
        "right click:",
        "    zoom out",
        "shift+drag (up|down):",
        "    change imaginary C",
        "shift+drag (left|right):",
        "    change real C"
    };
    
    glColor3f(1, 1, 1);
    for(i = lines - 1; i >= 0; i--){
        pos_y = WIN_INFO_H - 640 - i * 15;
        printtext(10, pos_y, str_mouse[i]);
    }
    
	glPopAttrib();
	glFlush();
}

void render ()
{
	double x = (double) width / tex_w;
    double y = (double) height / tex_h;
    
	glClear(GL_COLOR_BUFFER_BIT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
	glBindTexture(GL_TEXTURE_2D, texture);
    
	glBegin(GL_QUADS);
    
	glTexCoord2f(0, 0); glVertex2i(0, 0);
	glTexCoord2f(x, 0); glVertex2i(width, 0);
	glTexCoord2f(x, y); glVertex2i(width, height);
	glTexCoord2f(0, y); glVertex2i(0, height);
    
	glEnd();
    
	glFlush();
	glFinish();
}

int dump = 1;
void screen_dump ()
{
	char fn[100];
	int i;
	sprintf(fn, "screen%03d.ppm", dump++);
	FILE *fp = fopen(fn, "w");
	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	for (i = height - 1; i >= 0; i--) {
		fwrite(tex[i], 1, width * 3, fp);
    }
	fclose(fp);
	printf("%s written\n", fn);
}

void screen_dump_csv(){
	FILE * fp;
	int x, y;
	char *filename = "fractal.csv";
    
	fp = fopen(filename, "w");
    
	for (y = 0; y < WIN_DISP_H; y++) {
		fprintf(fp, "%d", iterations[y][x]);
        for (x = 0; x < WIN_DISP_W; x++) {
            fprintf(fp, ",%d", iterations[y][x]);
        }
        fprintf(fp, "\n");
	}
	fclose(fp);
}

void hsv_to_rgb (int hue, int min, int max, rgb_t *p)
{
	if (min == max) {
        max = min + 1;
    }
    
	if (invert) {
        hue = max - (hue - min);
    }
    
	if (!saturation) {
		p->r = p->g = p->b = 255 * (max - hue) / (max - min);
		return;
	}
    
	double h = fmod(color_rotate + 1e-4 + 4.0 * (hue - min) / (max - min), 6);
#	define VAL 255
	double c = VAL * saturation;
	double X = c * (1 - fabs(fmod(h, 2) - 1));
    
	p->r = p->g = p->b = 0;
    
	switch((int)h) {
        case 0:
            p->r = c;
            p->g = X;
            break;
        case 1:
            p->r = X;
            p->g = c;
            break;
        case 2:
            p->g = c;
            p->b = X;
            break;
        case 3:
            p->g = X;
            p->b = c;
            break;
        case 4:
            p->r = X;
            p->b = c;
            break;
        default:
            p->r = c;
            p->b = X;
            break;
	}
}

void *calc_mandel (void *p)
{
	int i, j, iter;
	rgb_t *px;
	double x, y, lenZ;
    
    double _Complex varZ;
	double _Complex varC;
    
    int blockSize = height / max_threads;
    int myThreadNum = *(int *) p;
    
    
	for (i = myThreadNum * blockSize; i < (myThreadNum + 1) * blockSize; i++)
    {
		px = tex[i];
		y = (i - height / 2) * scale + cy;
        
		for (j = 0; j < width; j++, px++)
        {
			x = (j - width / 2) * scale + cx;
			iter = 0;
            
            if (julia) {
                varZ = x + y * I;
                varC = vr + vi * I;
            } else {
                varZ = 0;
                varC = x + y * I;
            }
            
			for (iter = 0, lenZ = 0; iter < max_iter && lenZ < 4; iter++)
            {
                switch (fz) {
                    case 1:
                        varZ = (varZ * varZ) + varC;
                        break;
                    case 2:
                        varZ = cexp(cpow(varZ, 3)) + varC;
                    default:
                        break;
                }
                
				lenZ = creal(varZ) * creal(varZ) + cimag(varZ) * cimag(varZ);
			}
            
			if (iter < mandel_min) {
                mandel_min = iter;
            }
			
            if (iter > mandel_max) {
                mandel_max = iter;
            }
            
            iterations[i][j] = iter;
            if (iter < 0) {
                printf("Menor que cero\n");
            }
            
            *(unsigned short *)px = iter;
		}
	}
    
	for (i = myThreadNum * blockSize; i < (myThreadNum + 1) * blockSize; i++)
		for (j = 0, px = tex[i]; j < width; j++, px++)
			hsv_to_rgb(*(unsigned short *)px, mandel_min, mandel_max, px);
    
    pthread_exit(0);
}

void alloc_tex ()
{
	int i,
        ow = tex_w,
        oh = tex_h;
    
	for (tex_w = 1; tex_w < width;  tex_w <<= 1);
	for (tex_h = 1; tex_h < height; tex_h <<= 1);
    
	if (tex_h != oh || tex_w != ow)
		tex = realloc(tex, tex_h * tex_w * 3 + tex_h * sizeof(rgb_t*));
    
	for (tex[0] = (rgb_t *)(tex + tex_h), i = 1; i < tex_h; i++)
		tex[i] = tex[i - 1] + tex_w;
}

void set_texture ()
{
    int hThreads[max_threads];
	pthread_t tId[max_threads];
    int tNum[max_threads];
	int i;
    
	alloc_tex();
    
    mandel_max = 0;
    mandel_min = max_iter;
    
    // Create the threads
	for (i = 0; i < max_threads; i++) {
        tNum[i] = i;
		hThreads[i] = pthread_create(&tId[i], NULL, calc_mandel, &tNum[i]);
    }
    
    // Wait for all threads to finish
    for (i = 0; i < max_threads; i++) {
        pthread_join(tId[i], NULL);
    }
    
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, tex_w, tex_h, 0, GL_RGB, GL_UNSIGNED_BYTE, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glutSetWindow(gwin_display);
	render();
}

void keypress (unsigned char key, int x, int y)
{
	switch(key) {
            /* Reset */
        case 27: // esc
            scale = target_scale = 1./256;
            cx = target_cx = 0.0;
            cy = target_cy = 0.0;
            vi = target_vi;
            vr = target_vr;
            
            set_texture();
            glutSetWindow(gwin_info);
            display();
            break;
        
            /* Write image to PPM file */
        case 'p':
            screen_dump();
            return;
            
            /* Write image to CSV file */
        case 'o':
            screen_dump_csv();
            return;
        
            /* Max iterations */
        case 'x':
			target_max_iter += 128;
			if (target_max_iter > 1 << 15) {
                target_max_iter = 1 << 15;
            }
			break;
        case 'z':
			target_max_iter -= 128;
			if (target_max_iter < 128) {
                target_max_iter = 128;
            }
			break;
        case 'X':
			target_max_iter += 8;
			if (target_max_iter > 1 << 15) {
                target_max_iter = 1 << 15;
            }
			break;
        case 'Z':
			target_max_iter -= 8;
			if (target_max_iter < 128) {
                target_max_iter = 128;
            }
			break;
        
            /* Shift */
        case 'a':
            target_cx -= 50 * scale;
            break;
        case 'd':
            target_cx += 50 * scale;
            break;
        case 's':
            target_cy -= 50 * scale;
            break;
        case 'w':
            target_cy += 50 * scale;
            break;
        case 'A':
            target_cx -= 4 * scale;
            break;
        case 'D':
            target_cx += 4 * scale;
            break;
        case 'S':
            target_cy -= 4 * scale;
            break;
        case 'W':
            target_cy += 4 * scale;
            break;
        
            /* Scale */
        case 'q':
            target_scale *= 2;
            break;
        case 'e':
            target_scale /= 2;
            break;
        case 'Q':
            target_scale *= 1.05;
            break;
        case 'E':
            target_scale /= 1.05;
            break;
        
            /* Invert colors */
        case ' ':
            invert = !invert;
            set_texture();
            break;
            /* Monochromatic */
        case 'c':
            saturation = 1 - saturation;
            set_texture();
			break;
            /* Color rotation */
        case 'r':
            color_rotate = (color_rotate + 1) % 6;
            set_texture();
			break;
        
            /* Change fractal */
        case '1':
            fz = 1;
            julia = 0;
            
            vr = 0.0;
            vi = 0.0;
            target_vr = 0.0;
            target_vi = 0.0;
            
            scale = 1./256;
            cx = 0;
            cy = 0;
            max_iter = 128;
            set_texture();
            glutSetWindow(gwin_info);
            display();
            break;
        case '2':
            fz = 1;
            julia = 1;
            
            target_vr = 0.285;
            target_vi = 0.01;
            
            scale = 1./256;
            cx = 0;
            cy = 0;
            max_iter = 128;
            break;
        case '3':
            fz = 1;
            julia = 1;
            
            target_vr = 1.0 - 1.6180339887;
            target_vi = 0.0;
            
            scale = 1./256;
            cx = 0;
            cy = 0;
            max_iter = 128;
            break;
        case '4':
            fz = 1;
            julia = 1;
            
            target_vr = -0.8;
            target_vi = 0.156;
            
            scale = 1./256;
            cx = 0;
            cy = 0;
            max_iter = 128;
            break;
        case '5':
            fz = 2;
            julia = 1;
            
            target_vr = -0.621;
            target_vi = 0.0;
            
            scale = 1./256;
            cx = 0;
            cy = 0;
            max_iter = 128;
            break;
            
            /* Increment|decrement the constant */
        case 'h':
            target_vr *= 1.01;
            break;
        case 'f':
            target_vr /= 1.01;
            break;
        case 't':
            target_vi *= 1.01;
            break;
        case 'g':
            target_vi /= 1.01;
            break;
        case 'H':
            target_vr *= 1.001;
            break;
        case 'F':
            target_vr /= 1.001;
            break;
        case 'T':
            target_vi *= 1.005;
            break;
        case 'G':
            target_vi /= 1.005;
            break;
            
        default:
            break;
	}
}

void mouseclick (int button, int state, int x, int y)
{
	mouseX = x;
	mouseY = y;
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			if(glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
				valuemouse = 1;
				vrpivot = -(double) x / width * scale;
				vipivot = -(double) y / height * scale;
			} else {
				zoom = 1;
			}
        } else {
            zoom = 0;
            valuemouse = 0;
        }
	} else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN)
			zoom = -1;
		else
			zoom = 0;
	} else if (button == GLUT_MIDDLE_BUTTON) {
		if (state == GLUT_DOWN) {
			valuemouse = 1;
			vrpivot = -(double) x / width * scale;
			vipivot = -(double) y / height * scale;
		} else {
			valuemouse = 0;
		}
	}
}

void mousemove (int x, int y)
{
	mouseX = x;
	mouseY = y;
}

void mousedrag (int x, int y)
{
	mouseX = x;
	mouseY = y;
	/*if (valuemouse) {
	 target_vr = vrpivot + (float)x / XDIM * scale;
	 target_vi = vipivot + (float)y / YDIM * scale;
	 }*/
}

void animate ()
{
    double scale_diff = 0;
    double cx_diff = 0;
    double cy_diff = 0;
    double vr_diff = 0;
    double vi_diff = 0;
    
	if (zoom == 1) {
		target_scale /= 1.08;
	} else if (zoom == -1) {
		target_scale *= 1.08;
	}
	if (zoom == -1 || zoom == 1) {
		target_cx += (mouseX - width / 2.0) * scale / 1;
		target_cy -= (mouseY - height / 2.0) * scale / 1;
	}
	if (valuemouse) {
		if(!lock_vr) target_vr += (vrpivot + (double) mouseX / width * scale) / vprecision;
		if(!lock_vi) target_vi += (vipivot + (double) mouseY / height * scale) / vprecision;
	}
    
	vr += (target_vr - vr) / 8;
	vi += (target_vi - vi) / 8;
	cx = (target_cx - cx) / 8;
	cy = (target_cy - cy) / 8;
	scale += (target_scale - scale) / 4;
	max_iter += (target_max_iter - max_iter) / 2;
    
    scale_diff = fabs(fabs(scale) - fabs(target_scale));
    vr_diff = fabs(fabs(vr) - fabs(target_vr));
    vi_diff = fabs(fabs(vi) - fabs(target_vi));
    cx_diff = fabs(fabs(cx) - fabs(target_cx)) * scale;
    cy_diff = fabs(fabs(cy) - fabs(target_cy)) * scale;
    
    animation_stop = .01 * scale;
	if (scale_diff > animation_stop || vr_diff > animation_stop || vi_diff > animation_stop
        || cx_diff > animation_stop || cy_diff > animation_stop
        || target_max_iter != max_iter) {
        
		set_texture();
		glutPostRedisplay();
        
        glutSetWindow(gwin_info);
        display();
        
        ++render_num;
	}
    
//    printf("cx: %f\n", vr);
//    printf("tg: %f\n", target_vr);
//    printf("df: %f\n", vr_diff);
}

void resize (int w, int h)
{
	printf("resize %d %d\n", w, h);
	width = w;
	height = h;
    
	glViewport(0, 0, w, h);
	glOrtho(0, w, 0, h, -1, 1);
    
	set_texture();
}

void init_gfx (int *c, char **v)
{
	glutInit(c, v);
    // General windows settings
	glutInitDisplayMode(GLUT_RGB);
    
    // Info window settings
    glutInitWindowSize(WIN_INFO_W, WIN_INFO_H);
    glutInitWindowPosition(0, 0);
    // Create the window to display program and fractal information
    gwin_info = glutCreateWindow("Info");
    // Info window callbacks
	glutDisplayFunc(display);
    
    // Display window settings
	glutInitWindowSize(WIN_DISP_W, WIN_DISP_H);
    glutInitWindowPosition(WIN_INFO_W + 5, 0);
    // Create the window to display the fractals
    gwin_display = glutCreateWindow("Display");
    // Display window callbacks
	glutDisplayFunc(render);
	glutKeyboardFunc(keypress);
	glutMouseFunc(mouseclick);
    glutIdleFunc(animate);
    glutPassiveMotionFunc(mousemove);
    glutMotionFunc(mousedrag);
	glutReshapeFunc(resize);
	glGenTextures(1, &texture);
    // Generate the fractal
	set_texture();
}

int main (int c, char **v)
{
    max_threads = get_max_cpus();
    
	init_gfx(&c, v);
    
	glutMainLoop();
	return 0;
}