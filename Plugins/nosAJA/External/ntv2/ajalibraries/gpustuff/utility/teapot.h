/* SPDX-License-Identifier: MIT */
/*
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */
//
// teapot.h - Creates a display list that draws a teapot
//

GLuint teapotDisplayList;

long patchData[][16] = {
    {102,103,104,105,4,5,6,7,8,9,10,11,12,13,14,15},
    {12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27},
    {24,25,26,27,29,30,31,32,33,34,35,36,37,38,39,40},
    {96,96,96,96,97,98,99,100,101,101,101,101,0,1,2,3,},
    {0,1,2,3,106,107,108,109,110,111,112,113,114,115,116,117},
    {118,118,118,118,124,122,119,121,123,126,125,120,40,39,38,37},
    {41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56},
    {53,54,55,56,57,58,59,60,61,62,63,64,28,65,66,67},
    {68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83},
    {80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95}
};

double cpData[][3] = {
    {0.2,0,2.7},{0.2,-0.112,2.7},{0.112,-0.2,2.7},{0,-0.2,2.7},
    {1.3375,0,2.53125},{1.3375,-0.749,2.53125},{0.749,-1.3375,2.53125},
    {0,-1.3375,2.53125},{1.4375,0,2.53125},{1.4375,-0.805,2.53125},
    {0.805,-1.4375,2.53125},{0,-1.4375,2.53125},{1.5,0,2.4},{1.5,-0.84,2.4},
    {0.84,-1.5,2.4},{0,-1.5,2.4},{1.75,0,1.875},{1.75,-0.98,1.875},
    {0.98,-1.75,1.875},{0,-1.75,1.875},{2,0,1.35},{2,-1.12,1.35},
    {1.12,-2,1.35},{0,-2,1.35},{2,0,0.9},{2,-1.12,0.9},{1.12,-2,0.9},
    {0,-2,0.9},{-2,0,0.9},{2,0,0.45},{2,-1.12,0.45},{1.12,-2,0.45},
    {0,-2,0.45},{1.5,0,0.225},{1.5,-0.84,0.225},{0.84,-1.5,0.225},
    {0,-1.5,0.225},{1.5,0,0.15},{1.5,-0.84,0.15},{0.84,-1.5,0.15},
    {0,-1.5,0.15},{-1.6,0,2.025},{-1.6,-0.3,2.025},{-1.5,-0.3,2.25},
    {-1.5,0,2.25},{-2.3,0,2.025},{-2.3,-0.3,2.025},{-2.5,-0.3,2.25},
    {-2.5,0,2.25},{-2.7,0,2.025},{-2.7,-0.3,2.025},{-3,-0.3,2.25},
    {-3,0,2.25},{-2.7,0,1.8},{-2.7,-0.3,1.8},{-3,-0.3,1.8},{-3,0,1.8},
    {-2.7,0,1.575},{-2.7,-0.3,1.575},{-3,-0.3,1.35},{-3,0,1.35},
    {-2.5,0,1.125},{-2.5,-0.3,1.125},{-2.65,-0.3,0.9375},{-2.65,0,0.9375},
    {-2,-0.3,0.9},{-1.9,-0.3,0.6},{-1.9,0,0.6},{1.7,0,1.425},
    {1.7,-0.66,1.425},{1.7,-0.66,0.6},{1.7,0,0.6},{2.6,0,1.425},
    {2.6,-0.66,1.425},{3.1,-0.66,0.825},{3.1,0,0.825},{2.3,0,2.1},
    {2.3,-0.25,2.1},{2.4,-0.25,2.025},{2.4,0,2.025},{2.7,0,2.4},
    {2.7,-0.25,2.4},{3.3,-0.25,2.4},{3.3,0,2.4},{2.8,0,2.475},
    {2.8,-0.25,2.475},{3.525,-0.25,2.49375},{3.525,0,2.49375},
    {2.9,0,2.475},{2.9,-0.15,2.475},{3.45,-0.15,2.5125},{3.45,0,2.5125},
    {2.8,0,2.4},{2.8,-0.15,2.4},{3.2,-0.15,2.4},{3.2,0,2.4},{0,0,3.15},
    {0.8,0,3.15},{0.8,-0.45,3.15},{0.45,-0.8,3.15},{0,-0.8,3.15},
    {0,0,2.85},{1.4,0,2.4},{1.4,-0.784,2.4},{0.784,-1.4,2.4},{0,-1.4,2.4},
    {0.4,0,2.55},{0.4,-0.224,2.55},{0.224,-0.4,2.55},{0,-0.4,2.55},
    {1.3,0,2.55},{1.3,-0.728,2.55},{0.728,-1.3,2.55},{0,-1.3,2.55},
    {1.3,0,2.4},{1.3,-0.728,2.4},{0.728,-1.3,2.4},{0,-1.3,2.4},{0,0,0},
    {1.425,-0.798,0},{1.5,0,0.075},{1.425,0,0},{0.798,-1.425,0},
    {0,-1.5,0.075},{0,-1.425,0},{1.5,-0.84,0.075},{0.84,-1.5,0.075}
};


void createTeapot(long grid)
{
    GLdouble p[4][4][3], q[4][4][3], r[4][4][3], s[4][4][3];
    long i, j, k, l;

    teapotDisplayList = 1;
    glNewList(teapotDisplayList, GL_COMPILE);
    glPushMatrix();
    glRotatef(270.0, 1.0, 0.0, 0.0);
    for (i = 0; i < 10; i++) {
	for (j = 0; j < 4; j++) {
	    for (k = 0; k < 4; k++) {
		for (l = 0; l < 3; l++) {
		    p[j][k][l] = cpData[patchData[i][j*4+k]][l];
		    q[j][k][l] = cpData[patchData[i][j*4+(3-k)]][l];
		    if (l == 1) {
			q[j][k][l] *= -1.0;
		    }
		    if (i < 6) {
			r[j][k][l] = cpData[patchData[i][j*4+(3-k)]][l];
			if (l == 0) {
			    r[j][k][l] *= -1.0;
			}
			s[j][k][l] = cpData[patchData[i][j*4+k]][l];
			if (l == 0) {
			    s[j][k][l] *= -1.0;
			}
			if (l == 1) {
			    s[j][k][l] *= -1.0;
			}
		    }
		}
	    }
	}
	glMap2d(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &p[0][0][0]);
	glEnable(GL_MAP2_VERTEX_3);
	glMapGrid2f(grid, 0.0, 1.0, grid, 0.0, 1.0);
	glEvalMesh2(GL_FILL, 0, grid, 0, grid);
	glMap2d(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &q[0][0][0]);
	glEvalMesh2(GL_FILL, 0, grid, 0, grid);
	if (i < 6) {
	    glMap2d(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &r[0][0][0]);
	    glEvalMesh2(GL_FILL, 0, grid, 0, grid);
	    glMap2d(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &s[0][0][0]);
	    glEvalMesh2(GL_FILL, 0, grid, 0, grid);
	}
    }
    glDisable(GL_MAP2_VERTEX_3);
    glPopMatrix();
    glEndList();
}

void drawTeapot()
{
	glCallList(teapotDisplayList);
}