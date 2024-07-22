#define _CRT_SECURE_NO_WARNINGS

#include <gl/glut.h>   
#include <stdio.h>
#include <math.h>
#include <iostream>
using namespace std;

#include <stdlib.h>
#include <stdio.h>

#define	PI	3.1415926
#define		N	36
#define		M	18

float	radius, theta, phi;
float	offset = 3.0;
float	plus_value = 1.0;
GLUquadricObj* obj;
float	light_position[4] = { 7.0, 0.0, 5.0, 1.0 };
float	light_ambient[4] = { 0.0, 1.0, 0.0, 1.0 }; // 주변광(은은하게 배이는 색)
float	light_diffuse[4] = { 1.0, 0.0, 0.0, 1.0 }; // 난반사 빛의 색깔
float	light_specular[4] = { 0.0, 1.0, 1.0, 1.0 }; // 정반사 빛의 색깔

const float	pin_specular[4] = { 1.0, 1.0, 1.0, 1.0 }; // 핀의 색깔
const float	pin_ambient[4] = { 1.0, 0.0, 1.0, 1.0 };
const float	pin_diffuse[4] = { 1.0, 1.0, 0.0, 1.0 };

float	face_position[4] = { 7.0, 0.0, 5.0, 1.0 };
float	face_specular[4] = { 1.0, 1.0, 1.0, 1.0 }; // 면의 색깔
float	face_ambient[4] = { 1.0, 0.0, 1.0, 1.0 }; // 면의 색깔
float	face_diffuse[4] = { 0.0, 1.0, 1.0, 1.0 }; // 면의 색깔
float	pinhead_specular[4] = { 1.0, 0.0, 0.0, 1.0 };

float	r = 0.5;
float	h = 3.0;

float	ver_up_circle[N + 1][M + 1][3];    // 원기둥 윗면
float	ver_down_circle[N + 1][M + 1][3];  // 원기둥 아랫면
float	ver_face[N + 1][M + 2][3];         // 원기둥 옆면
float	normal_vec[3];                     // 노말벡터 구하기용
float	X[4], Y[4], Z[4];                  // 노말벡터 구하기용

float	ver[N + 1][M + 1][3]; // 공의 좌표
float	ball_r = 1.0;         // 공의 반지름
float	delta_theta, delta_phi;
float	start_theta, start_phi;

float	mouse_index_x = 25.0, mouse_index_y = 0.0;
float	ball_speed = 0.0;
//float	ball_index_x = 25.0;
//float	ball_index_y = 0.0;
bool	ball_check = true; // 공이 충돌하였는가
bool	pin_check = true; // 핀이 쓰러지는가
bool	d_check = true;   // 화면이 전환되었는가
struct pin_Index
{
	float ver[N + 1][M + 2][3];
	float normal_vector[N + 1][M + 2][3];
	float x;			// 위치
	float y;
	float z;
	float speed_x;		// 속도
	float speed_y;
	float speed_z;
	float a_o_r;		// 회전각
	bool check_collsion;// 충돌여부
};
const int pin_num = 10;
pin_Index index[10]; // 원기둘 옆면에 대한 좌표와 노말벡터들을 저장하기 위함
pin_Index down[10]; // 원기둘 밑면에 대한 좌표와 노말벡터들을 저장하기 위함

pin_Index ball;

GLuint	texture;

BITMAPINFOHEADER	bitmapInfoHeader;
unsigned char* bitmapImage;

unsigned char* LoadBitmapFile(const char* filename, BITMAPINFOHEADER* bitmapInfoHeader)
{
	FILE* filePtr;
	BITMAPFILEHEADER	bitmapFileHeader;
	unsigned char* bitmapImage;
	int					imageIdx = 0;
	unsigned char		tempRGB;

	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return NULL;

	// 비트맵 파일 헤드를 읽는다.
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	// 윈도우즈 BMP 파일인지 확인한다. 
	if (bitmapFileHeader.bfType != 'MB') {
		fclose(filePtr);
		return	(NULL);
	}

	// 비트맵 정보 헤드를 읽는다.
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	// 파일 포인터를 비트맵 데이터의 시작 위치로 옮긴다. 
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// 비트맵 이미지 데이터를 넣을 메모리를 할당한다. 
	bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

	//메모리가 제대로 할당되었는지 확인 
	if (!bitmapImage) {
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}

	// 메모리에 비트맵 데이트를 읽어드림 
	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);

	// 제대로 들어갔는지 확인 
	if (bitmapImage == NULL) {
		fclose(filePtr);
		return NULL;
	}

	//BMP는 BGR형식이므로 R과 B를 서로 교체해야 함
	for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;


		//		bitmapImage[imageIdx+1] = 0; 
		//		bitmapImage[imageIdx+2] = 0; 
	}

	// 파일을 닫고 비트맵 이미지 데이터 반환 
	fclose(filePtr);
	return	bitmapImage;
}

// 핀의 색깔 빨강, 정반사 빛의 색깔 파랑 = 난반사광의 색깔
void index_init() // 볼링핀의 좌표를 적어 놓은 것.
{
	index[0].x = down[0].x = 0;
	index[0].y = down[0].y = 0;
	index[0].z = down[0].z = 0;

	index[1].x = down[1].x = 1.5;
	index[1].y = down[1].y = -0.6;
	index[1].z = down[1].z = 0;

	index[2].x = down[2].x = 1.5;
	index[2].y = down[2].y = 0.6;
	index[2].z = down[2].z = 0;

	index[3].x = down[3].x = 3;
	index[3].y = down[3].y = -1.2;
	index[3].z = down[3].z = 0;

	index[4].x = down[4].x = 3;
	index[4].y = down[4].y = 0;
	index[4].z = down[4].z = 0;

	index[5].x = down[5].x = 3;
	index[5].y = down[5].y = 1.2;
	index[5].z = down[5].z = 0;

	index[6].x = down[6].x = 4.5;
	index[6].y = down[6].y = -2.0;
	index[6].z = down[6].z = 0;

	index[7].x = down[7].x = 4.5;
	index[7].y = down[7].y = -0.7;
	index[7].z = down[7].z = 0;

	index[8].x = down[8].x = 4.5;
	index[8].y = down[8].y = 0.7;
	index[8].z = down[8].z = 0;

	index[9].x = down[9].x = 4.5;
	index[9].y = down[9].y = 2.0;
	index[9].z = down[9].z = 0;

}

void ball_init()
{
	start_theta = 0.0;
	delta_theta = 2.0 * PI / N;

	start_phi = -1.0 * PI / 2.0;
	delta_phi = 1.0 * PI / M;
	float tht; // 카메라와 같이 연결된 세타이기에 공의 위치를 바꾸면 카메라 시점이 초기화
	float pi;  // 해결하기위해 새로운 변수에 저장
	for (int j = 0; j <= M; j++) {			// phi (위도)
		for (int i = 0; i <= N; i++) { 		// theta (경도)
			tht = start_theta + i * delta_theta;
			pi = start_phi + j * delta_phi;
			ball.ver[i][j][0] = mouse_index_x + ball_speed + ball_r * cos(pi) * cos(tht);
			ball.ver[i][j][1] = mouse_index_y + ball_r * cos(pi) * sin(tht);
			ball.ver[i][j][2] = 1 + ball_r * sin(pi);
		}
	}
}

void external_func(int i, int j) {

	float ver_dir[4][3] = {};
	float ver_area[4][3] = {};
	float ver_avg[4] = {};
	float ver_unit[4][3] = {};
	float ver_ext[3] = {};

	// * 고쳐야 할 것  * /

	//if (j == 0) {
	//	j = 1;
	//}
	//if (j == 18) {
	//	j = 17;
	//}

	//  점을 기준으로 사분면 방향 벡터 구하는 과정

	ver_dir[0][0] = ball.ver[(i + 1 + N) % N][j][0] - ball.ver[i][j][0];  //right
	ver_dir[0][1] = ball.ver[(i + 1 + N) % N][j][1] - ball.ver[i][j][1];
	ver_dir[0][2] = ball.ver[(i + 1 + N) % N][j][2] - ball.ver[i][j][2];

	ver_dir[1][0] = ball.ver[i][j + 1][0] - ball.ver[i][j][0]; //up
	ver_dir[1][1] = ball.ver[i][j + 1][1] - ball.ver[i][j][1];
	ver_dir[1][2] = ball.ver[i][j + 1][2] - ball.ver[i][j][2];

	ver_dir[2][0] = ball.ver[(i - 1 + N) % N][j][0] - ball.ver[i][j][0]; //left
	ver_dir[2][1] = ball.ver[(i - 1 + N) % N][j][1] - ball.ver[i][j][1];
	ver_dir[2][2] = ball.ver[(i - 1 + N) % N][j][2] - ball.ver[i][j][2];

	ver_dir[3][0] = ball.ver[i][j - 1][0] - ball.ver[i][j][0]; //down
	ver_dir[3][1] = ball.ver[i][j - 1][1] - ball.ver[i][j][1];
	ver_dir[3][2] = ball.ver[i][j - 1][2] - ball.ver[i][j][2];

	// 사분면 벡터를 이용하여 법선 벡터를 구하는 과정 

	ver_area[0][0] = ver_dir[0][1] * ver_dir[1][2]
		- ver_dir[0][2] * ver_dir[1][1];
	ver_area[0][1] = ver_dir[0][2] * ver_dir[1][0]
		- ver_dir[0][0] * ver_dir[1][2];
	ver_area[0][2] = ver_dir[0][0] * ver_dir[1][1]
		- ver_dir[0][1] * ver_dir[1][0];

	ver_area[1][0] = ver_dir[1][1] * ver_dir[2][2] - ver_dir[1][2] * ver_dir[2][1];
	ver_area[1][1] = ver_dir[1][2] * ver_dir[2][0] - ver_dir[1][0] * ver_dir[2][2];
	ver_area[1][2] = ver_dir[1][0] * ver_dir[2][1] - ver_dir[1][1] * ver_dir[2][0];

	ver_area[2][0] = ver_dir[2][1] * ver_dir[3][2] - ver_dir[2][2] * ver_dir[3][1];
	ver_area[2][1] = ver_dir[2][2] * ver_dir[3][0] - ver_dir[2][0] * ver_dir[3][2];
	ver_area[2][2] = ver_dir[2][0] * ver_dir[3][1] - ver_dir[2][1] * ver_dir[3][0];

	ver_area[3][0] = ver_dir[3][1] * ver_dir[0][2] - ver_dir[3][2] * ver_dir[0][1];
	ver_area[3][1] = ver_dir[3][2] * ver_dir[0][0] - ver_dir[3][0] * ver_dir[0][2];
	ver_area[3][2] = ver_dir[3][0] * ver_dir[0][1] - ver_dir[3][1] * ver_dir[0][0];

	// 단위벡터를 구하기 위한 법선 벡터의 크기 

	ver_avg[0] = sqrt(ver_area[0][0] * ver_area[0][0]
		+ ver_area[0][1] * ver_area[0][1] + ver_area[0][2] * ver_area[0][2]);
	ver_avg[1] = sqrt(ver_area[1][0] * ver_area[1][0]
		+ ver_area[1][1] * ver_area[1][1] + ver_area[1][2] * ver_area[1][2]);
	ver_avg[2] = sqrt(ver_area[2][0] * ver_area[2][0]
		+ ver_area[2][1] * ver_area[2][1] + ver_area[2][2] * ver_area[2][2]);
	ver_avg[3] = sqrt(ver_area[3][0] * ver_area[3][0]
		+ ver_area[3][1] * ver_area[3][1] + ver_area[3][2] * ver_area[3][2]);

	// 단위벡터 구하는 과정 

	ver_unit[0][0] = ver_area[0][0] / ver_avg[0];
	ver_unit[0][1] = ver_area[0][1] / ver_avg[0];
	ver_unit[0][2] = ver_area[0][2] / ver_avg[0];

	ver_unit[1][0] = ver_area[1][0] / ver_avg[1];
	ver_unit[1][1] = ver_area[1][1] / ver_avg[1];
	ver_unit[1][2] = ver_area[1][2] / ver_avg[1];

	ver_unit[2][0] = ver_area[2][0] / ver_avg[2];
	ver_unit[2][1] = ver_area[2][1] / ver_avg[2];
	ver_unit[2][2] = ver_area[2][2] / ver_avg[2];

	ver_unit[3][0] = ver_area[3][0] / ver_avg[3];
	ver_unit[3][1] = ver_area[3][1] / ver_avg[3];
	ver_unit[3][2] = ver_area[3][2] / ver_avg[3];

	// 단위 벡터들의 평균을 구하는 과정 

	ball.normal_vector[i][j][0] = (ver_unit[0][0] + ver_unit[1][0]
		+ ver_unit[2][0] + ver_unit[3][0]) / 4;
	ball.normal_vector[i][j][1] = (ver_unit[0][1] + ver_unit[1][1]
		+ ver_unit[2][1] + ver_unit[3][1]) / 4;
	ball.normal_vector[i][j][2] = (ver_unit[0][2] + ver_unit[1][2]
		+ ver_unit[2][2] + ver_unit[3][2]) / 4;

	//return ver_ext[0], ver_ext[1], ver_ext[2];
}


float external_down_func(void) {

	float ver_dir[36][3] = {};
	float ver_area[36][3] = {};
	float ver_avg[36] = {};
	float ver_unit[36][3] = {};
	float ver_ext[3] = { 0, 0, 0 };

	for (int k = 0; k < 36; k++) {
		ver_dir[k][0] = ver[k][1][0] - ver[k][0][0];
		ver_dir[k][1] = ver[k][1][1] - ver[k][0][1];
		ver_dir[k][2] = ver[k][1][2] - ver[k][0][2];
	}

	// 사분면 벡터를 이용하여 법선 벡터를 구하는 과정 

	for (int m = 0; m < 36; m++) {
		ver_area[m][0] = -(ver_dir[m][1] * ver_dir[(m + 1) % N][2]
			- ver_dir[m][2] * ver_dir[(m + 1) % N][1]);
		ver_area[m][1] = -(ver_dir[m][2] * ver_dir[(m + 1) % N][0]
			- ver_dir[m][0] * ver_dir[(m + 1) % N][2]);
		ver_area[m][2] = -(ver_dir[m][0] * ver_dir[(m + 1) % N][1]
			- ver_dir[m][1] * ver_dir[(m + 1) % N][0]);
	}

	// 단위벡터를 구하기 위한 법선 벡터의 크기 

	for (int n = 0; n < 36; n++) {
		ver_avg[n] = sqrt(ver_area[n][0] * ver_area[n][0]
			+ ver_area[n][1] * ver_area[n][1] + ver_area[n][2] * ver_area[n][2]);
	}

	// 단위벡터 구하는 과정 
	for (int s = 0; s < 36; s++) {
		ver_unit[s][0] = ver_area[s][0] / ver_avg[s];
		ver_unit[s][1] = ver_area[s][1] / ver_avg[s];
		ver_unit[s][2] = ver_area[s][2] / ver_avg[s];
	}

	// 단위 벡터들의 평균을 구하는 과정 

	for (int t = 0; t < 36; t++) {
		ver_ext[0] += ver_unit[t][0];
		ver_ext[1] += ver_unit[t][1];
		ver_ext[2] += ver_unit[t][2];
	}

	ver_ext[0] = ver_ext[0] / 36;
	ver_ext[1] = ver_ext[1] / 36;
	ver_ext[2] = ver_ext[2] / 36;

	return ver_ext[0], ver_ext[1], ver_ext[2];
}


float external_up_func(void) {

	float ver_dir[36][3] = {};
	float ver_area[36][3] = {};
	float ver_avg[36] = {};
	float ver_unit[36][3] = {};
	float ver_ext[3] = { 0,0,0 };

	for (int k = 0; k < 36; k++) {
		ver_dir[k][0] = ver[k][17][0] - ver[k][18][0];
		ver_dir[k][1] = ver[k][17][1] - ver[k][18][1];
		ver_dir[k][2] = ver[k][17][2] - ver[k][18][2];
	}

	// 사분면 벡터를 이용하여 법선 벡터를 구하는 과정 

	for (int m = 0; m < 36; m++) {
		ver_area[m][0] = (ver_dir[m][1] * ver_dir[(m + 1) % N][2]
			- ver_dir[m][2] * ver_dir[(m + 1) % N][1]);
		ver_area[m][1] = (ver_dir[m][2] * ver_dir[(m + 1) % N][0]
			- ver_dir[m][0] * ver_dir[(m + 1) % N][2]);
		ver_area[m][2] = (ver_dir[m][0] * ver_dir[(m + 1) % N][1]
			- ver_dir[m][1] * ver_dir[(m + 1) % N][0]);
	}

	// 단위벡터를 구하기 위한 법선 벡터의 크기 

	for (int n = 0; n < 36; n++) {
		ver_avg[n] = sqrt(ver_area[n][0] * ver_area[n][0]
			+ ver_area[n][1] * ver_area[n][1] + ver_area[n][2] * ver_area[n][2]);
	}

	// 단위벡터 구하는 과정 
	for (int s = 0; s < 36; s++) {
		ver_unit[s][0] = ver_area[s][0] / ver_avg[s];
		ver_unit[s][1] = ver_area[s][1] / ver_avg[s];
		ver_unit[s][2] = ver_area[s][2] / ver_avg[s];
	}

	// 단위 벡터들의 평균을 구하는 과정 

	for (int t = 0; t < 36; t++) {
		ver_ext[0] += ver_unit[t][0];
		ver_ext[1] += ver_unit[t][1];
		ver_ext[2] += ver_unit[t][2];
	}

	ver_ext[0] = ver_ext[0] / 36;
	ver_ext[1] = ver_ext[1] / 36;
	ver_ext[2] = ver_ext[2] / 36;

	return ver_ext[0], ver_ext[1], ver_ext[2];
}


void Drawball(void) { // 공 그리기

	float	ver_ex[3] = {};

	for (int j = 0; j < M; j++) {
		for (int i = 0; i < N; i++) {
			glBegin(GL_POLYGON);

			if (j == 0)
				ver_ex[0], ver_ex[1], ver_ex[2] = external_down_func();
			else
				external_func(i, j);
			glNormal3fv(ball.normal_vector[i][j]);
			glVertex3fv(ball.ver[i][j]);

			if (j == 0)
				ver_ex[0], ver_ex[1], ver_ex[2] = external_down_func();
			else
				external_func((i + 1) % N, j);
			glNormal3fv(ball.normal_vector[(i + 1) % N][j]);
			glVertex3fv(ball.ver[(i + 1) % N][j]);

			if (j + 1 == 18)
				ver_ex[0], ver_ex[1], ver_ex[2] = external_up_func();
			else
				external_func((i + 1) % N, j + 1);
			glNormal3fv(ball.normal_vector[(i + 1) % N][j + 1]);
			glVertex3fv(ball.ver[(i + 1) % N][j + 1]);


			if (j + 1 == 18)
				ver_ex[0], ver_ex[1], ver_ex[2] = external_up_func();
			else
				external_func(i, j + 1);
			glNormal3fv(ball.normal_vector[i][j + 1]);
			glVertex3fv(ball.ver[i][j + 1]);

			glEnd();
		}
	}

}

void init(void)
{
	glClearColor(0.5, 0.5, 0.5, 0.0);
	//glColor3f(1.0, 1.0, 0.0);
	radius = 30.0;
	theta = 0.0;
	phi = 0.0;

	index_init();                                    // 볼링핀의 시작 좌표들을 만드는 함수
	ball_init();
	for (int i = 0; i < 10; i++)
	{
		index[i].check_collsion = true;
		if (index[i].y + mouse_index_y > 0) { // 충돌할 때 날라가는 방향을 속도로 설정
			index[i].speed_y = 0.1;
			down[i].speed_y = 0.02;
		}
		else {
			index[i].speed_y = -0.1;
			down[i].speed_y = -0.02;
		}
		index[i].speed_x = -0.3;      // 핀의 옆면과 밑면을 설정
		down[i].speed_x = -0.1;
		index[i].speed_z = 0.1;
		down[i].speed_z = 0.02;
		index[i].a_o_r = -0.02;
		down[i].a_o_r = -0.02;
		down[i].check_collsion = true;
	}
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST); // Z-buffer 사용, 카메라로 부터 멀리있는 것 부터 정렬 
	glEnable(GL_CULL_FACE);  //후면 제거모드 https://sune.tistory.com/272
	glFrontFace(GL_CCW);     // 반시계 방향으로 그려진것을 전면으로 한다.

	glEnable(GL_TEXTURE_2D);

	bitmapImage = LoadBitmapFile("marble.bmp", &bitmapInfoHeader); // 파일 읽어오기

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // 텍스처 매핑하면서 이미지보다 큰 영역을 할때 반복해서 적용
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //텍스처 좌표에 가장 가까운 픽셀의 색상값을 선택
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmapInfoHeader.biWidth,
		bitmapInfoHeader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmapImage);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	//glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT1, GL_POSITION, face_position);
	glLightfv(GL_LIGHT1, GL_AMBIENT, face_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, face_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, face_specular);

}
//https://heinleinsgame.tistory.com/9

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 1.0, 100.0);
}

void axis(void) {
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); // x축 
	glVertex3f(-50.0, 0.0, 0.0);
	glVertex3f(50.0, 0.0, 0.0);

	glColor3f(0.0, 1.0, 0.0); // y축 
	glVertex3f(0.0, -50.0, 0.0);
	glVertex3f(0.0, 50.0, 0.0);

	glColor3f(0.0, 0.0, 1.0); // z축 
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 10.0);
	glEnd();
}

void camera(void) {
	float	x, y, z;

	x = radius * cos(theta) * cos(phi);
	y = radius * sin(theta) * cos(phi);
	z = radius * sin(phi);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
}

void Normal_Vector_Compliation(float ver1[4], float ver2[4], float ver3[4], float normal[3]) // 주어진 상하좌우의 점들의 성분들로 법선벡터 구하기
{
	float X[12] = { 0.0 , };
	float unit[12] = { 0.0, };
	//cout << ver1[3] << " " << ver2[3] << " " << ver3[3] << '\n';
	X[0] = ver2[0] * ver3[1] - ver3[0] * ver2[1]; // x
	X[1] = ver3[0] * ver1[1] - ver1[0] * ver3[1]; // y
	X[2] = ver1[0] * ver2[1] - ver2[0] * ver1[1]; // z

	X[3] = ver2[1] * ver3[2] - ver3[1] * ver2[2]; // x
	X[4] = ver3[1] * ver1[2] - ver1[1] * ver3[2]; // y
	X[5] = ver1[1] * ver2[2] - ver2[1] * ver1[2]; // z

	X[6] = ver2[2] * ver3[3] - ver3[2] * ver2[3]; // x
	X[7] = ver3[2] * ver1[3] - ver1[2] * ver3[3]; // y
	X[8] = ver1[2] * ver2[3] - ver2[2] * ver1[3]; // z

	X[9] = ver2[3] * ver3[0] - ver3[3] * ver2[0]; // x
	X[10] = ver3[3] * ver1[0] - ver1[3] * ver3[0]; // y
	X[11] = ver1[3] * ver2[0] - ver2[3] * ver1[0]; // z

	float avg = sqrt(X[0] * X[0] + X[1] * X[1] + X[2] * X[2]);
	float avg2 = sqrt(X[3] * X[3] + X[4] * X[4] + X[5] * X[5]);
	float avg3 = sqrt(X[6] * X[6] + X[7] * X[7] + X[8] * X[8]);
	float avg4 = sqrt(X[9] * X[9] + X[10] * X[10] + X[11] * X[11]);

	unit[0] = X[0] / avg;
	unit[1] = X[1] / avg;
	unit[2] = X[2] / avg;

	unit[3] = X[3] / avg2;
	unit[4] = X[4] / avg2;
	unit[5] = X[5] / avg2;

	unit[6] = X[6] / avg3;
	unit[7] = X[7] / avg3;
	unit[8] = X[8] / avg3;

	unit[9] = X[9] / avg4;
	unit[10] = X[10] / avg4;
	unit[11] = X[11] / avg4;


	normal[0] = (unit[0] + unit[3] + unit[6] + unit[9]) / 4;
	normal[1] = (unit[1] + unit[4] + unit[7] + unit[10]) / 4;
	normal[2] = (unit[2] + unit[5] + unit[8] + unit[11]) / 4;
}

void make_index(int i, int j, int k)
{
	float ver[N + 1][M + 1][3] = { 0, };
	if (j == 0) { j = 1; }
	if (j == 18) { j = 17; }

	switch (k)
	{
	case 0:
		for (int j = 0; j <= M; j++) // 원기둥 밑면을 이루는 점을 기준으로함
		{
			for (int i = 0; i <= N; i++)
			{
				for (int k = 0; k < 3; k++)
				{
					ver[i][j][k] = ver_down_circle[i][j][k];
				}
			}
		}
		break;
	case 1:
		for (int j = 0; j <= M; j++) // 원기둥의 옆면을 이루는 점을 기준으로함
		{
			for (int i = 0; i <= N; i++)
			{
				for (int k = 0; k < 3; k++)
				{
					ver[i][j][k] = ver_face[i][j][k];
				}
			}
		}
		break;
	case 2:
		for (int j = 0; j <= M; j++) // 원기둥 윗면을 이루는 점을 기준으로함.
		{
			for (int i = 0; i <= N; i++)
			{
				for (int k = 0; k < 3; k++)
				{
					ver[i][j][k] = ver_up_circle[i][j][k];
				}
			}
		}
		break;
	default:
		break;
	}
	//cout << ver_face[0][1][0] << " " << ver_face[0][1][1] << " " << ver_face[0][1][2] << "\n";
	//cout << ver[0][1][0] << " " << ver[0][1][1] << " " << ver[0][1][2] << "\n";
	X[0] = ver[(i + 1) % N][j][0] - ver[i][j][0];      // right의 x좌표
	X[1] = ver[i][j + 1][0] - ver[i][j][0];           // up
	X[2] = ver[(i + N - 1) % N][j][0] - ver[i][j][0]; // left
	X[3] = ver[i][j - 1][0] - ver[i][j][0];             // down



	Y[0] = ver[(i + 1) % N][j][1] - ver[i][j][1];             // right의 y좌표
	Y[1] = ver[i][j + 1][1] - ver[i][j][1];             // up
	Y[2] = ver[(i + N - 1) % N][j][1] - ver[i][j][1];   // left
	Y[3] = ver[i][j - 1][1] - ver[i][j][1];             // down



	Z[0] = ver[(i + 1) % N][j][2] - ver[i][j][2];             // right의 z좌표
	Z[1] = ver[i][j + 1][2] - ver[i][j][2];             // up
	Z[2] = ver[(i + N - 1) % N][j][2] - ver[i][j][2];   // left
	Z[3] = ver[i][j - 1][2] - ver[i][j][2];             // down


}

void cylinder()
{
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	GLfloat x, y, z, start_angle, delta_angle, angle;

	start_angle = 0.0;
	delta_angle = 2.0f * PI / N;


	// 원기둥 아랫면을 만들기 위함
	for (int j = 0; j <= M; j++) // 원을 이루는 점을 M 등분 하기위함
	{
		for (int i = 0; i <= N; i++) // 원을 이루는 점을 기반
		{
			angle = start_angle + i * delta_angle;

			ver_down_circle[i][j][0] = 0 + (r / M) * j * sin(angle);
			ver_down_circle[i][j][1] = 0 + (r / M) * j * cos(angle);
			ver_down_circle[i][j][2] = 0;

			//make_index(i, j, 0);
			//Normal_Vector_Compliation(X, Y, Z, normal_vec);
			//glNormal3fv(normal_vec);

			//x = center[0] + (r)*sin(angle); // 시계방향으로 만들기
			//y = center[1] + (r)*cos(angle);
			//z = 0;

		}

	}



	//원기둥의 옆면

	for (int j = 0; j <= M; j++) // 원을 이루는 점을 M 등분 하기위함
	{

		for (int i = 0; i <= N; i++) // 원을 이루는 점을 기반
		{
			angle = start_angle + i * delta_angle;
			//index[k].ver[i][j][0] = index[k].x + (r / M) * j * sin(angle);
			//index[k].ver[i][j][1] = index[k].y + (r / M) * j * sin(angle);
			//index[k].ver[i][j][2] = (h / M) * j;


			ver_face[i][j][0] = 0 + (r / M) * M * sin(angle);
			ver_face[i][j][1] = 0 + (r / M) * M * cos(angle);
			ver_face[i][j][2] = (h / M) * j;

			//x = center[0] + (r)*sin(angle); // 시계방향으로 만들기
			//y = center[1] + (r)*cos(angle);
			//z = 0;
		}
	}


	/*
	glBegin(GL_QUAD_STRIP);
	glColor3f(1.0, 0.0, 0.0);
	for (int i = 0; i <= N; i++) // 빨간부분
	{
		angle = start_angle + i * delta_angle;
		x = center[0] + (r+0.01)* sin(angle);
		y = center[1] + (r+0.01) * cos(angle);
		z = h;
		glVertex3f(x, y, h - 1);
		glVertex3f(x, y, h - 0.5);
	}
	glEnd();
	*/


	/*
	glBegin(GL_POLYGON);           //원기둥의 윗면
	//glNormal3f(0.0f, 0.0f, 1.0f);
	//glVertex3f(center[0], center[1], center[2] + h);
	for (int j = 0; j <= M; j++) // 원을 이루는 점을 M 등분 하기위함
	{
		glVertex3f(center[0], center[1], center[2] + h); // 원점 x, y, z 좌표
		for (int i = 0; i <= N; i++) // 원을 이루는 점을 기반
		{
			angle = start_angle + i * delta_angle;
			ver_up_circle[i][j][0] = center[0] + (r / M) * j * sin(angle);
			ver_up_circle[i][j][1] = center[1] + (r / M) * j * cos(angle);
			ver_up_circle[i][j][2] = (center[2] + h);
			//x = center[0] + (r)*sin(angle); // 시계방향으로 만들기
			//y = center[1] + (r)*cos(angle);
			//z = 0;
			glVertex3fv(ver_up_circle[i][j]);
		}

	}
	glEnd();

	//glDisable(GL_LIGHT0);
	//glDisable(GL_LIGHTING);
	*/
}
//https://evir.tistory.com/entry/OpenGL%EA%B0%84%EB%8B%A8%ED%95%9C-%EC%9B%90%EA%B8%B0%EB%91%A5-%EB%A7%8C%EB%93%9C%EB%8A%94-%ED%95%A8%EC%88%98

void save_normal(int i, int j, float normal_v[3], float v[][M + 2][3], int check_num) // accept_normal()에서 구한 법선벡터와 점의 위치들을 저장
{
	if (check_num == 0)
	{
		cout << "숫자를 제대로 입력해 주십시오";
	}
	if (check_num == 1)
	{
		for (int k = 0; k < pin_num; k++)
		{
			index[k].normal_vector[i][j][0] = normal_vec[0];
			index[k].normal_vector[i][j][1] = normal_vec[1];
			index[k].normal_vector[i][j][2] = normal_vec[2];

			index[k].ver[i][j][0] = ver_face[i][j][0];
			index[k].ver[i][j][1] = ver_face[i][j][1];
			index[k].ver[i][j][2] = ver_face[i][j][2];
		}
	}
	if (check_num == 2)
	{
		for (int k = 0; k < pin_num; k++)
		{
			index[k].normal_vector[(i + 1) % N][j][0] = normal_vec[0];
			index[k].normal_vector[(i + 1) % N][j][1] = normal_vec[1];
			index[k].normal_vector[(i + 1) % N][j][2] = normal_vec[2];

			index[k].ver[(i + 1) % N][j][0] = ver_face[(i + 1) % N][j][0];
			index[k].ver[(i + 1) % N][j][1] = ver_face[(i + 1) % N][j][1];
			index[k].ver[(i + 1) % N][j][2] = ver_face[(i + 1) % N][j][2];
		}
	}
	if (check_num == 3)
	{
		for (int k = 0; k < pin_num; k++)
		{
			index[k].normal_vector[(i + 1) % N][j + 1][0] = normal_vec[0];
			index[k].normal_vector[(i + 1) % N][j + 1][1] = normal_vec[1];
			index[k].normal_vector[(i + 1) % N][j + 1][2] = normal_vec[2];

			index[k].ver[(i + 1) % N][j + 1][0] = ver_face[(i + 1) % N][j + 1][0];
			index[k].ver[(i + 1) % N][j + 1][1] = ver_face[(i + 1) % N][j + 1][1];
			index[k].ver[(i + 1) % N][j + 1][2] = ver_face[(i + 1) % N][j + 1][2];
		}
	}
	if (check_num == 4)
	{
		for (int k = 0; k < pin_num; k++)
		{
			index[k].normal_vector[i][j + 1][0] = normal_vec[0];
			index[k].normal_vector[i][j + 1][1] = normal_vec[1];
			index[k].normal_vector[i][j + 1][2] = normal_vec[2];

			index[k].ver[i][j + 1][0] = ver_face[i][j + 1][0];
			index[k].ver[i][j + 1][1] = ver_face[i][j + 1][1];
			index[k].ver[i][j + 1][2] = ver_face[i][j + 1][2];
		}
	}
	if (check_num == 5)
	{
		for (int k = 0; k < pin_num; k++)
		{
			down[k].normal_vector[i][j][0] = normal_vec[0];
			down[k].normal_vector[i][j][1] = normal_vec[1];
			down[k].normal_vector[i][j][2] = normal_vec[2];

			down[k].ver[i][j][0] = ver_down_circle[i][j][0];
			down[k].ver[i][j][1] = ver_down_circle[i][j][1];
			down[k].ver[i][j][2] = ver_down_circle[i][j][2];
		}
	}
	if (check_num == 6)
	{
		for (int k = 0; k < pin_num; k++)
		{
			down[k].normal_vector[(i + 1) % N][j][0] = normal_vec[0];
			down[k].normal_vector[(i + 1) % N][j][1] = normal_vec[1];
			down[k].normal_vector[(i + 1) % N][j][2] = normal_vec[2];

			down[k].ver[(i + 1) % N][j][0] = ver_down_circle[(i + 1) % N][j][0];
			down[k].ver[(i + 1) % N][j][1] = ver_down_circle[(i + 1) % N][j][1];
			down[k].ver[(i + 1) % N][j][2] = ver_down_circle[(i + 1) % N][j][2];
		}
	}
	if (check_num == 7)
	{
		for (int k = 0; k < pin_num; k++)
		{
			down[k].normal_vector[(i + 1) % N][j + 1][0] = normal_vec[0];
			down[k].normal_vector[(i + 1) % N][j + 1][1] = normal_vec[1];
			down[k].normal_vector[(i + 1) % N][j + 1][2] = normal_vec[2];

			down[k].ver[(i + 1) % N][j + 1][0] = ver_down_circle[(i + 1) % N][j + 1][0];
			down[k].ver[(i + 1) % N][j + 1][1] = ver_down_circle[(i + 1) % N][j + 1][1];
			down[k].ver[(i + 1) % N][j + 1][2] = ver_down_circle[(i + 1) % N][j + 1][2];
		}
	}
	if (check_num == 8)
	{
		for (int k = 0; k < pin_num; k++)
		{
			down[k].normal_vector[i][j + 1][0] = normal_vec[0];
			down[k].normal_vector[i][j + 1][1] = normal_vec[1];
			down[k].normal_vector[i][j + 1][2] = normal_vec[2];

			down[k].ver[i][j + 1][0] = ver_down_circle[i][j + 1][0];
			down[k].ver[i][j + 1][1] = ver_down_circle[i][j + 1][1];
			down[k].ver[i][j + 1][2] = ver_down_circle[i][j + 1][2];
		}
	}
}

void accept_normal()
{
	glMaterialfv(GL_FRONT, GL_SPECULAR, pin_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, pin_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, pin_specular);
	glPushMatrix();
	//glTranslatef(center[0], center[1], center[2]);
	//glRotatef(45, 0.0, 0.0, 0.0);
	int num = 0;                                              // 기준점의 어느부분을 구했는지 알리기 위한 flag
	//cout << ver_face[0][1][0] << " " << ver_face[0][1][1] << " " << ver_face[0][1][2] << "\n";
	for (int j = 0; j < M; j++)                               // 원기둥의 옆면을 구하는 용도         
	{
		for (int i = 0; i <= N; i++)
		{

			//glBegin(GL_POLYGON);
			make_index(i, j, 1);                              // 기준 점 x,y좌표와 면을 구성하는 점임을 알리기 위한 정수
			Normal_Vector_Compliation(X, Y, Z, normal_vec);   // 면을 구성하는 점의 법선벡터 구하기
			//glNormal3fv(normal_vec);
			//glVertex3fv(ver_face[i][j]);
			save_normal(i, j, normal_vec, ver_face, 1);       // 면의 좌표와 법선벡터의 좌표를 저장하기위함 , 1은 기준점 윗점의 성분을 구했다는 것을 표현 

			make_index((i + 1) % N, j, 1);
			Normal_Vector_Compliation(X, Y, Z, normal_vec);
			//glNormal3fv(normal_vec);
			//glVertex3fv(ver_face[(i + 1) % N][j]);
			save_normal(i, j, normal_vec, ver_face, 2);

			make_index((i + 1) % N, j + 1, 1);
			Normal_Vector_Compliation(X, Y, Z, normal_vec);
			//glNormal3fv(normal_vec);
			//glVertex3fv(ver_face[(i + 1) % N][j+1]);

			save_normal(i, j, normal_vec, ver_face, 3);

			make_index(i, j + 1, 1);
			Normal_Vector_Compliation(X, Y, Z, normal_vec);
			//glNormal3fv(normal_vec);
			//glVertex3fv(ver_face[i][j + 1]);

			save_normal(i, j, normal_vec, ver_face, 4);
			//glEnd();

		}
	}


	for (int j = 0; j < M; j++)                   // 원기둥 밑면(원)을 구하기위함.
	{
		for (int i = 0; i <= N; i++)
		{
			//glBegin(GL_POLYGON);
			make_index(i, j, 0);                  // 기준 점 x,y좌표와 면을 구성하는 점임을 알리기 위한 정수
			Normal_Vector_Compliation(X, Y, Z, normal_vec); // 원을 구성하는 점의 법선벡터 구하기
			//glNormal3fv(normal_vec);
			//glVertex3fv(ver_down_circle[i][j]);
			save_normal(i, j, normal_vec, ver_face, 5); // 원의 좌표와 법선벡터의 좌표를 저장하기위함 , 5는 기준점 윗점의 성분을 구했다는 것을 표현 


			make_index((i + 1) % N, j, 0);
			Normal_Vector_Compliation(X, Y, Z, normal_vec);
			//glNormal3fv(normal_vec);
			//glVertex3fv(ver_down_circle[(i + 1) % N][j]);
			save_normal(i, j, normal_vec, ver_face, 6);



			make_index((i + 1) % N, j + 1, 0);
			Normal_Vector_Compliation(X, Y, Z, normal_vec);
			//glNormal3fv(normal_vec);
			//glVertex3fv(ver_down_circle[(i + 1) % N][j + 1]);
			save_normal(i, j, normal_vec, ver_face, 7);


			make_index(i, j + 1, 0);
			Normal_Vector_Compliation(X, Y, Z, normal_vec);
			//glNormal3fv(normal_vec);
			//glVertex3fv(ver_down_circle[i][j + 1]);
			save_normal(i, j, normal_vec, ver_face, 8);

			//glEnd();
		}
	}
	//glRotatef(45, 0.0, 0.0, 0.0);
	//glTranslatef(-center[0], -center[1], -center[2]);
	glPopMatrix();
}
/*
void bowling_pin() // 컴퓨터에서 주어지는 함수로 원기둥 만들기
{
	obj = gluNewQuadric();
	gluQuadricDrawStyle(obj, GLU_FILL); // 물체의 형태를 솔리드 형태로
	gluQuadricNormals(obj, GLU_SMOOTH); // 물체의 법선벡터를 부드럽게
	gluQuadricOrientation(obj, GLU_OUTSIDE); //법선벡터를 물체의 바깥은 향하게
	gluQuadricTexture(obj, GL_FALSE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);


	glPushMatrix();
	//glMaterialfv(GL_FRONT, GL_SPECULAR, pin_specular);
	//glColor3f(1.0, 1.0, 1.0); init()에서 lightfv 다 뺀 경우
	gluCylinder(obj, 0.5, 0.5, 3, 32, 4); // 물체,아래반지름,윗반지름,높이,조각수
	glTranslatef(0.0, 0.0, 2.0);

	//glMaterialfv(GL_FRONT, GL_SPECULAR, pinhead_specular);
	//glColor3f(1.0, 0.0, 0.0);
	gluCylinder(obj, 0.51, 0.51, 0.5, 32, 4);
	glTranslatef(0.0, 0.0, 1.0);

	//glMaterialfv(GL_FRONT, GL_SPECULAR, pin_specular);
	gluDisk(obj, 0.0, 0.5, 32, 4);
	glTranslatef(0.0, 0.0, -3.0);

	glPopMatrix();
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

}
*/
//https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=nanapig94&logNo=221157038461
//http://www.gisdeveloper.co.kr/?p=35

/*
void init_collision() // 원기둥 초기화
{
	init();
	//for (int i = 0; i < 10; i++)
		//index[i].check_collsion = true;
}
*/

/*
void pin_trans_rotate(int i)  // 핀의 회전각을 바꿔 쓰러지는 모습을 보여줄려고 했던 함수
{
	glPushMatrix();
	glTranslatef(index[i].x, index[i].y, index[i].z);
	glRotatef(90, 0, 1, 0);
	for (int j = 0; j < M; j++)
	{
		for (int k = 0; k <= N; k++)
		{
			glBegin(GL_POLYGON);
			glNormal3fv(index[i].normal_vector[k][j]);
			glVertex3fv(index[i].ver[k][j]);


			glNormal3fv(index[i].normal_vector[(k + 1) % N][j]);
			glVertex3fv(index[i].ver[(k + 1) % N][j]);


			glNormal3fv(index[i].normal_vector[(k + 1) % N][j + 1]);
			glVertex3fv(index[i].ver[(k + 1) % N][j + 1]);


			glNormal3fv(index[i].normal_vector[k][j + 1]);
			glVertex3fv(index[i].ver[k][j + 1]);

			glEnd();
		}
	}
	glRotatef(90, 0, -1, 0);
	glTranslatef(-index[i].x, -index[i].y, -index[i].z);
	glPopMatrix();
}
*/

void pins(bool check)
{
	if (check == true)
	{
		glPushMatrix();
		for (int k = 0; k < pin_num; k++)                     // 모든 핀(원기둥)들의 옆면에 대한 구성성분들(노말벡터와 점의 좌표들)을 다른 자리에 놓기위해 glTranslatef() 사용  
		{

			glTranslatef(index[k].x, index[k].y, index[k].z);

			for (int j = 0; j < M; j++)
			{
				for (int i = 0; i <= N; i++)
				{
					glBegin(GL_POLYGON);
					glNormal3fv(index[k].normal_vector[i][j]);
					glVertex3fv(index[k].ver[i][j]);


					glNormal3fv(index[k].normal_vector[(i + 1) % N][j]);
					glVertex3fv(index[k].ver[(i + 1) % N][j]);


					glNormal3fv(index[k].normal_vector[(i + 1) % N][j + 1]);
					glVertex3fv(index[k].ver[(i + 1) % N][j + 1]);


					glNormal3fv(index[k].normal_vector[i][j + 1]);
					glVertex3fv(index[k].ver[i][j + 1]);

					glEnd();
				}
			}
			glTranslatef(-index[k].x, -index[k].y, -index[k].z);

			/*
			for (int j = 0; j < M; j++)
			{
				for (int i = 0; i <= N; i++)            // 핀의 노말벡터와 좌표를 미리 설정 해둔 핀의 좌표로 옮기고 싶어서 만든것
				{                                       // 기존에는 구조체에 핀의 중심을 설정안하여 이런식으로 계산하려고 하다 실패함.

						index[k].normal_vector[i][j][0] = index[k].normal_vector[i][j][0] + index[k].x;
						index[k].normal_vector[i][j][1] = index[k].normal_vector[i][j][1] + index[k].y;
						index[k].normal_vector[i][j][2] = index[k].normal_vector[i][j][2] + index[k].z;
						index[k].ver[i][j][0] = index[k].ver[i][j][0] + index[k].x;
						index[k].ver[i][j][1] = index[k].ver[i][j][1] + index[k].y;
						index[k].ver[i][j][2] = index[k].ver[i][j][2] + index[k].z;


						index[k].normal_vector[(i + 1) % N][j][0] = index[k].normal_vector[(i + 1) % N][j][0] + index[k].x;
						index[k].normal_vector[(i + 1) % N][j][1] = index[k].normal_vector[(i + 1) % N][j][1] + index[k].y;
						index[k].normal_vector[(i + 1) % N][j][2] = index[k].normal_vector[(i + 1) % N][j][2] + index[k].z;
						index[k].ver[(i + 1) % N][j][0] = index[k].ver[(i + 1) % N][j][0] + index[k].x;
						index[k].ver[(i + 1) % N][j][1] = index[k].ver[(i + 1) % N][j][1] + index[k].y;
						index[k].ver[(i + 1) % N][j][2] = index[k].ver[(i + 1) % N][j][2] + index[k].z;

						index[k].normal_vector[(i + 1) % N][j + 1][0] = index[k].normal_vector[(i + 1) % N][j + 1][0] + index[k].x;
						index[k].normal_vector[(i + 1) % N][j + 1][1] = index[k].normal_vector[(i + 1) % N][j + 1][1] + index[k].y;
						index[k].normal_vector[(i + 1) % N][j + 1][2] = index[k].normal_vector[(i + 1) % N][j + 1][2] + index[k].z;
						index[k].ver[(i + 1) % N][j + 1][0] = index[k].ver[(i + 1) % N][j + 1][0] + index[k].x;
						index[k].ver[(i + 1) % N][j + 1][1] = index[k].ver[(i + 1) % N][j + 1][1] + index[k].y;
						index[k].ver[(i + 1) % N][j + 1][2] = index[k].ver[(i + 1) % N][j + 1][2] + index[k].z;


						index[k].normal_vector[i][j + 1][0] = index[k].normal_vector[i][j + 1][0] + index[k].x;
						index[k].normal_vector[i][j + 1][1] = index[k].normal_vector[i][j + 1][1] + index[k].y;
						index[k].normal_vector[i][j + 1][2] = index[k].normal_vector[i][j + 1][2] + index[k].z;
						index[k].ver[i][j + 1][0] = index[k].ver[i][j + 1][0] + index[k].x;
						index[k].ver[i][j + 1][1] = index[k].ver[i][j + 1][1] + index[k].y;
						index[k].ver[i][j + 1][2] = index[k].ver[i][j + 1][2] + index[k].z;

				}
			}
			*/

			glPopMatrix();
			glPushMatrix();
			for (int k = 0; k < pin_num; k++)                                         // 핀(원기둥)을 이루는 밑면에 대한 성분(노말벡터와 점의좌표)들을 각각 다른 위치에 놓기 위함.
			{
				glTranslatef(down[k].x, down[k].y, down[k].z);
				for (int j = 0; j <= M; j++) // 원을 이루는 점을 M 등분 하기위함
				{
					for (int i = 0; i <= N; i++) // 원을 이루는 점을 기반
					{
						glBegin(GL_POLYGON);
						glNormal3fv(down[k].normal_vector[i][j]);
						glVertex3fv(down[k].ver[i][j]);


						glNormal3fv(down[k].normal_vector[(i + 1) % N][j]);
						glVertex3fv(down[k].ver[(i + 1) % N][j]);


						glNormal3fv(down[k].normal_vector[(i + 1) % N][j + 1]);
						glVertex3fv(down[k].ver[(i + 1) % N][j + 1]);


						glNormal3fv(down[k].normal_vector[i][j + 1]);
						glVertex3fv(down[k].ver[i][j + 1]);

						glEnd();
					}
				}
				glTranslatef(-down[k].x, -down[k].y, -down[k].z);
			}

			//glTranslatef(1, -0.6, 0); // 원기둥 중심 좌표를 바꾸었을 때 노말벡터가 기존의 것과 변하는지 실험.
			//cylinder();
			//accept_normal();
			//glTranslatef(-1, 0.6, 0);

			//glTranslatef(1, +0.6, 0);
			//cylinder();
			//accept_normal();
			//glTranslatef(-1, -0.6, 0);
			glPopMatrix();
		}
	}
	else //충돌했을 때
	{
		glPushMatrix();
		for (int k = 0; k < pin_num; k++)                     // 모든 핀(원기둥)들의 옆면에 대한 구성성분들(노말벡터와 점의 좌표들)을 다른 자리에 놓기위해 glTranslatef() 사용  
		{
			glTranslatef(index[k].x, index[k].y, index[k].z);

			if (index[k].check_collsion == false)             // 10개의 핀중 충돌이 일어난 핀에 대해 핀이 밀려나가는 부분을 설정
			{
				glRotatef(index[k].a_o_r, 0, -1, 0);
				//index[k].a_o_r = index[k].a_o_r + index[k].a_o_r; // 회전각을 설정 하려고 했지만 뜻대로 나오지 않아 주석 처리
				index[k].x = index[k].x + index[k].speed_x;
				index[k].y = index[k].y + index[k].speed_y;
				index[k].z = index[k].z + index[k].speed_z;
			}

			for (int j = 0; j < M; j++)
			{
				for (int i = 0; i <= N; i++)
				{
					glBegin(GL_POLYGON);
					glNormal3fv(index[k].normal_vector[i][j]);
					glVertex3fv(index[k].ver[i][j]);


					glNormal3fv(index[k].normal_vector[(i + 1) % N][j]);
					glVertex3fv(index[k].ver[(i + 1) % N][j]);


					glNormal3fv(index[k].normal_vector[(i + 1) % N][j + 1]);
					glVertex3fv(index[k].ver[(i + 1) % N][j + 1]);


					glNormal3fv(index[k].normal_vector[i][j + 1]);
					glVertex3fv(index[k].ver[i][j + 1]);

					glEnd();
				}
			}
			if (index[k].check_collsion == false) // 충돌 했을 핀에 대해 회전 시키는 부분
			{
				glRotatef(index[k].a_o_r, 0, 1, 0);
			}
			glTranslatef(-index[k].x, -index[k].y, -index[k].z);

			/*
			for (int j = 0; j < M; j++)
			{
				for (int i = 0; i <= N; i++) // 날아가는 핀에 대해 원기둥 점과 노말벡터를 다시 구해야 한다고 생각해 적은 부분.
				{

						index[k].normal_vector[i][j][0] = index[k].normal_vector[i][j][0] + index[k].x;
						index[k].normal_vector[i][j][1] = index[k].normal_vector[i][j][1] + index[k].y;
						index[k].normal_vector[i][j][2] = index[k].normal_vector[i][j][2] + index[k].z;
						index[k].ver[i][j][0] = index[k].ver[i][j][0] + index[k].x;
						index[k].ver[i][j][1] = index[k].ver[i][j][1] + index[k].y;
						index[k].ver[i][j][2] = index[k].ver[i][j][2] + index[k].z;


						index[k].normal_vector[(i + 1) % N][j][0] = index[k].normal_vector[(i + 1) % N][j][0] + index[k].x;
						index[k].normal_vector[(i + 1) % N][j][1] = index[k].normal_vector[(i + 1) % N][j][1] + index[k].y;
						index[k].normal_vector[(i + 1) % N][j][2] = index[k].normal_vector[(i + 1) % N][j][2] + index[k].z;
						index[k].ver[(i + 1) % N][j][0] = index[k].ver[(i + 1) % N][j][0] + index[k].x;
						index[k].ver[(i + 1) % N][j][1] = index[k].ver[(i + 1) % N][j][1] + index[k].y;
						index[k].ver[(i + 1) % N][j][2] = index[k].ver[(i + 1) % N][j][2] + index[k].z;

						index[k].normal_vector[(i + 1) % N][j + 1][0] = index[k].normal_vector[(i + 1) % N][j + 1][0] + index[k].x;
						index[k].normal_vector[(i + 1) % N][j + 1][1] = index[k].normal_vector[(i + 1) % N][j + 1][1] + index[k].y;
						index[k].normal_vector[(i + 1) % N][j + 1][2] = index[k].normal_vector[(i + 1) % N][j + 1][2] + index[k].z;
						index[k].ver[(i + 1) % N][j + 1][0] = index[k].ver[(i + 1) % N][j + 1][0] + index[k].x;
						index[k].ver[(i + 1) % N][j + 1][1] = index[k].ver[(i + 1) % N][j + 1][1] + index[k].y;
						index[k].ver[(i + 1) % N][j + 1][2] = index[k].ver[(i + 1) % N][j + 1][2] + index[k].z;


						index[k].normal_vector[i][j + 1][0] = index[k].normal_vector[i][j + 1][0] + index[k].x;
						index[k].normal_vector[i][j + 1][1] = index[k].normal_vector[i][j + 1][1] + index[k].y;
						index[k].normal_vector[i][j + 1][2] = index[k].normal_vector[i][j + 1][2] + index[k].z;
						index[k].ver[i][j + 1][0] = index[k].ver[i][j + 1][0] + index[k].x;
						index[k].ver[i][j + 1][1] = index[k].ver[i][j + 1][1] + index[k].y;
						index[k].ver[i][j + 1][2] = index[k].ver[i][j + 1][2] + index[k].z;

				}
			}
			*/

			glPopMatrix();
			glPushMatrix();
			for (int k = 0; k < pin_num; k++)                                         // 핀(원기둥)을 이루는 밑면에 대한 성분(노말벡터와 점의좌표)들을 각각 다른 위치에 놓기 위함.
			{
				glTranslatef(down[k].x, down[k].y, down[k].z);

				if (index[k].check_collsion == false)					// 밑면에 대한 충돌. 위의 원기둥 옆면 충돌과 동일하게 적용
				{
					glRotatef(down[k].a_o_r, 0, -1, 0);
					//down[k].a_o_r = down[k].a_o_r + down[k].a_o_r;
					down[k].x = down[k].x + down[k].speed_x;
					down[k].y = down[k].y + down[k].speed_y;
					down[k].z = down[k].z + down[k].speed_z;
				}

				for (int j = 0; j <= M; j++) // 원을 이루는 점을 M 등분 하기위함
				{
					for (int i = 0; i <= N; i++) // 원을 이루는 점을 기반
					{
						glBegin(GL_POLYGON);
						glNormal3fv(down[k].normal_vector[i][j]);
						glVertex3fv(down[k].ver[i][j]);


						glNormal3fv(down[k].normal_vector[(i + 1) % N][j]);
						glVertex3fv(down[k].ver[(i + 1) % N][j]);


						glNormal3fv(down[k].normal_vector[(i + 1) % N][j + 1]);
						glVertex3fv(down[k].ver[(i + 1) % N][j + 1]);


						glNormal3fv(down[k].normal_vector[i][j + 1]);
						glVertex3fv(down[k].ver[i][j + 1]);

						glEnd();
					}
				}
				if (index[k].check_collsion == false)
				{
					glRotatef(down[k].a_o_r, 0, 1, 0);
				}
				glTranslatef(-down[k].x, -down[k].y, -down[k].z);
			}
			//glTranslatef(1, -0.6, 0);						// 원기둥 중심 좌표를 바꾸었을 때 노말벡터가 기존의 것과 변하는지 실험.
			//cylinder();
			//accept_normal();
			//glTranslatef(-1, 0.6, 0);

			//glTranslatef(1, +0.6, 0);
			//cylinder();
			//accept_normal();
			//glTranslatef(-1, -0.6, 0);
			glPopMatrix();
		}
		/*
		glPushMatrix();
		for (int k = 0; k < pin_num; k++)                     // 모든 핀(원기둥)들의 옆면에 대한 구성성분들(노말벡터와 점의 좌표들)을 다른 자리에 놓기위해 glTranslatef() 사용
		{
			for (int j = 0; j < M; j++)
			{
				for (int i = 0; i <= N; i++)
				{
					//if (j == 0) { j = 1; }
					//if (j == 18) { j = 17; }
					//index[k].normal_vector[i][j][0] = index[k].normal_vector[i][j][0] + index[k].x;
					//index[k].normal_vector[i][j][1] = index[k].normal_vector[i][j][1] + index[k].y;
					//index[k].normal_vector[i][j][2] = index[k].normal_vector[i][j][2] + index[k].z;
					index[k].ver[i][j][0] = index[k].ver[i][j][0] + index[k].x;
					index[k].ver[i][j][1] = index[k].ver[i][j][1] + index[k].y;
					index[k].ver[i][j][2] = index[k].ver[i][j][2] + index[k].z;


					//index[k].normal_vector[(i + 1) % N][j][0] = index[k].normal_vector[(i + 1) % N][j][0] + index[k].x;
					//index[k].normal_vector[(i + 1) % N][j][1] = index[k].normal_vector[(i + 1) % N][j][1] + index[k].y;
					//index[k].normal_vector[(i + 1) % N][j][2] = index[k].normal_vector[(i + 1) % N][j][2] + index[k].z;
					index[k].ver[(i + 1) % N][j][0] = index[k].ver[(i + 1) % N][j][0] + index[k].x;
					index[k].ver[(i + 1) % N][j][1] = index[k].ver[(i + 1) % N][j][1] + index[k].y;
					index[k].ver[(i + 1) % N][j][2] = index[k].ver[(i + 1) % N][j][2] + index[k].z;

					//index[k].normal_vector[(i + 1) % N][j + 1][0] = index[k].normal_vector[(i + 1) % N][j + 1][0] + index[k].x;
					//index[k].normal_vector[(i + 1) % N][j + 1][1] = index[k].normal_vector[(i + 1) % N][j + 1][1] + index[k].y;
					//index[k].normal_vector[(i + 1) % N][j + 1][2] = index[k].normal_vector[(i + 1) % N][j + 1][2] + index[k].z;
					index[k].ver[(i + 1) % N][j + 1][0] = index[k].ver[(i + 1) % N][j + 1][0] + index[k].x;
					index[k].ver[(i + 1) % N][j + 1][1] = index[k].ver[(i + 1) % N][j + 1][1] + index[k].y;
					index[k].ver[(i + 1) % N][j + 1][2] = index[k].ver[(i + 1) % N][j + 1][2] + index[k].z;


					//index[k].normal_vector[i][j + 1][0] = index[k].normal_vector[i][j + 1][0] + index[k].x;
					//index[k].normal_vector[i][j + 1][1] = index[k].normal_vector[i][j + 1][1] + index[k].y;
					//index[k].normal_vector[i][j + 1][2] = index[k].normal_vector[i][j + 1][2] + index[k].z;
					index[k].ver[i][j + 1][0] = index[k].ver[i][j + 1][0] + index[k].x;
					index[k].ver[i][j + 1][1] = index[k].ver[i][j + 1][1] + index[k].y;
					index[k].ver[i][j + 1][2] = index[k].ver[i][j + 1][2] + index[k].z;
					//cout << "k = " << k << " ver[i][j] = " << index[k].ver[i][j][0] << " " << index[k].ver[i][j][1] << " " << index[k].ver[i][j][2] << '\n';

				}
			}
		}

		glPopMatrix();
		*/
	}
}

void ball_pin_collision(int i, int k)
{
	float ball_x = mouse_index_x + ball_speed;
	float ball_y = mouse_index_y;
	float radi = ball_r + r;

	float distance[N + 1] = { 0, };
	float distance_x[N + 1] = { 0, };
	float distance_y[N + 1] = { 0, };
	start_theta = 0.0;
	delta_theta = 2.0 * PI / N;

	start_phi = -1.0 * PI / 2.0;
	delta_phi = 1.0 * PI / M;

	const int y = M / 2;												// 공의 경도중 중간을 설정

	//for (int j = 0; j <= M; j++) {			// phi (위도)			// 함수 내부에서 계산하려다가 너무 랙걸려서 밖으로 빼도록 함. 
	//for (int k = 0; k <= N; k++) { 		// theta (경도)
	//	for (int l = 0; l <= N; l++) {
			//tht = start_theta + i * delta_theta;
			//pi = start_phi + y * delta_phi;

	distance_x[k] = ball.ver[k][y][0] - index[i].x; // 공과 핀의 거리차이 (x)
	distance_x[k] = distance_x[k] * distance_x[k];
	distance_y[k] = ball.ver[k][y][1] - index[i].y; // 공과 핀의 거리차이 (y)
	distance_y[k] = distance_y[k] * distance_y[k];
	distance[k] = sqrt(distance_x[k] + distance_y[k]);

	//cout << "ball x:" << ball.ver[k][y][0] << '\n'; //= mouse_index_x + ball_speed + ball_r * cos(pi) * cos(tht);
	//cout << "ball y:" << ball.ver[k][y][1] << '\n'; //= mouse_index_y + ball_r * cos(pi) * sin(tht);
	//cout << "ball z:" << ball.ver[k][y][2] << '\n'; //= 1 + ball_r * sin(pi);
//	}
//}
//}
//for (int k = 0; k <= N; k++) { 		// theta (경도)
//	for (int l = 0; l <= N; l++) {
	if (distance[k] < radi)							// 핀이 충돌한 경우
	{
		//cout << "i = " << i << " k = " << k << '\n';
		//cout << " 공 x =" << ball.ver[k][y][0] << " 핀 x =" << index[i].x << '\n';
		//cout << " 공 y =" << ball.ver[k][y][1] << " 핀 y =" << index[i].y << '\n';
		//cout << " 공 z =" << ball.ver[k][y][2] << " 핀 z =" << index[i].z << '\n';
		pin_check = false;							// 핀들 중에 하나 이상이 충돌했다는 것을 알림. 
		//pin_trans_rotate(i);
		index[i].check_collsion = false;			// 10개의 핀중 해당 핀이 충돌했다는 것을 알림 
	}
	//	}
	//}

}


/*
void set_angle()						회전각을 설정하려고 만들었던 함수
{
	for (int i = 0; i < pin_num; i++)
	{
		index[i].a_o_r += 0.1;
		down[i].a_o_r += 0.1;
	}
}
*/


void d_camera(void) {
	float	x, y, z;

	x = mouse_index_x + ball_speed;
	y = mouse_index_y;//radius * sin(theta) * cos(phi);
	z = 1;//radius * sin(phi);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
}

void Drawface()
{
	glPushMatrix();

	glBegin(GL_QUADS);  // 윗면 

	glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, -5.0f, -0.01f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(30.0f, -5.0f, -0.01f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(30.0f, 5.0f, -0.01f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, 5.0f, -0.01f);

	glEnd();
	/*
	glBegin(GL_QUADS);  // 앞면
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
	glTexCoord2f(0.5f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.5f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glEnd();

	glBegin(GL_QUADS);  // 오른쪽 면
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, 0.5f, -0.5f);
	glTexCoord2f(5.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(5.0f, 1.0f); glVertex3f(0.5f, -0.5f, 0.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, -0.5f, -0.5f);
	glEnd();

	glBegin(GL_QUADS);  // 왼쪽 면
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glTexCoord2f(3.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
	glTexCoord2f(3.0f, 3.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.0f, 3.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
	glEnd();

	glBegin(GL_QUADS);  // 아랫 면
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
	glTexCoord2f(4.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glTexCoord2f(4.0f, 4.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
	glTexCoord2f(0.0f, 4.0f); glVertex3f(0.5f, -0.5f, -0.5f);
	glEnd();

	glBegin(GL_QUADS);  // 뒷 면


	glTexCoord2f(0.5f, 0.5f); glVertex3f(0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.5f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
	glTexCoord2f(0.0f, 0.5f); glVertex3f(-0.5f, 0.5f, -0.5f);

	glEnd();
	*/

	glPopMatrix();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (d_check == false)
		d_camera();
	else
		camera();
	glDisable(GL_LIGHTING);
	axis();
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position); // 빛고정

	//bowling_pin();
	//glDisable(GL_LIGHTING);

	//glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);								// 면을 그릴 때에는 다른 빞을 사용
	Drawface();
	glDisable(GL_LIGHT1);

	glEnable(GL_LIGHT0);
	//glDisable(GL_LIGHTING);

	glEnable(GL_LIGHTING);

	cylinder();                                          // cylinder() 함수를 통해 원기둥을 이루는 좌표들을 구하고
	accept_normal();                                     // accept_normal() 함수를 통해 원기둥을 이루는 좌표들의 법선벡터를 구한다.

	pins(pin_check);                                              // 이 때 구한 점의 좌표와 법선벡터를 통해 원기둥을 위치만 바꿔서 복사 붙여넣기를 한다. 

	ball_init();
	Drawball();
	for (int i = 0; i < pin_num; i++) // 공의 반지름 
	{
		for (int k = 0; k <= N; k++) { 		// theta (경도)

			if (index[i].check_collsion == true)
			{
				ball_pin_collision(i, k);
			}

		}
	}
	//pins(pin_check);
	//cout << index[0].a_o_r << " " << pin_check << " " << index[0].check_collsion << '\n'; // 공이 충돌 했을 때 잘 되는지 확인
	//set_angle(); 
	//ball_init();
	//cout << mouse_index_x + ball_speed << '\n';
	//cout << normal_vec[0] << " " << normal_vec[1] << " " << normal_vec[2] << '\n';
	glFlush();
	glutSwapBuffers();
}

void special_key(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:		theta -= 0.01;
		break;
	case GLUT_KEY_RIGHT:	theta += 0.01;
		break;
	case GLUT_KEY_UP:		phi += 0.01;
		break;
	case GLUT_KEY_DOWN:		phi -= 0.01;
		break;
	default: break;
	}
	glutPostRedisplay();
}

void mytimer(int value)
{
	if (ball_speed > -30 && ball_check == false)
	{
		ball_speed -= 0.3;
		//cout << ball_speed << '\n';
	}
	for (int i = 0; i < pin_num; i++)
	{
		if (index[i].check_collsion == false || down[i].check_collsion == false) {		// 충돌이 일어났을 때 회전각을 타이머를 이용해 증가시키기
			index[i].a_o_r += 0.1;
			down[i].a_o_r += 0.1;
		}
	}
	glutPostRedisplay();
	glutTimerFunc(40, mytimer, 1);
}


void Mykey(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a':				radius += 0.5;
		break;
	case 's':				radius -= 0.5;
		break;
	case 'r':				mouse_index_x = 25.0, ball_speed = 0, ball_check = true, pin_check = true, d_check = true, init();
		break;
	case 'p':				ball_check = false;//,glutTimerFunc(40, mytimer, 1);//ball_speed -= 0.1;
		break;
	case 'd':				if (d_check == true) d_check = false; else d_check = true;
		break;
	default: break;
	}
	glutPostRedisplay();
	//cout << ball_check << '\n';
}

void mouse_F(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) // 마우스 왼쪽클릭으로 볼링공 위치 설정
	{
		//mouse_index_x = x / 25;
		//ball_speed = 4.0;
		mouse_index_y = x / 80 - 4;
		//cout << mouse_index_x << " " << mouse_index_y << " " << x << '\n';

		ball_init();
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) // 오른쪽 클릭으로 공이 이동하는걸 적용. 이 때 애니메이션 적용전이었기에 확인용으로 넣었던 것
	{
		//ball_init();
		//mouse_index_x = x / 25;
		ball_speed -= 4.0;
		//cout << mouse_index_x << " " << mouse_index_y << " " << x << '\n';
		ball_init();
	}
}



void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("볼링프로그램");
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special_key);
	glutKeyboardFunc(Mykey);
	glutMouseFunc(mouse_F);
	glutTimerFunc(40, mytimer, 1);
	glutIdleFunc(display);
	glutMainLoop();
}

