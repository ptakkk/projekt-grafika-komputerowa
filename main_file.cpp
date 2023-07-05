/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "ground.h"
#include "rock.h"
#include "plant1.h"
#include "coral.h"
#include "fish1.h"
#include "fish2.h"
#include "glass.h"


float speed1 = PI/2;
float speed2 = PI / 3;
float aspectRatio=1;

bool isWPressed = false;
bool isSPressed = false;
bool isAPressed = false;
bool isDPressed = false;
bool isEPressed = false;
bool isQPressed = false;
float cameraX = 0;
float cameraZ = -50;
float cameraAngleY = 0;		
int cameraSpeed = 10;
int cameraAngleSpeed = 2;

ShaderProgram *sp;
ShaderProgram *spcol;
GLuint groundTex;
GLuint cubeTex;
GLuint sandTex;
GLuint plant1Tex;
GLuint rockTex;
GLuint coralTex;
GLuint fish1Tex;
GLuint fish2Tex;

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
    if (action==GLFW_PRESS) {
        
		if (key == GLFW_KEY_W) isWPressed = true;
		if (key == GLFW_KEY_S) isSPressed = true;
		if (key == GLFW_KEY_A) isAPressed = true;
		if (key == GLFW_KEY_D) isDPressed = true;
		if (key == GLFW_KEY_E) isEPressed = true;
		if (key == GLFW_KEY_Q) isQPressed = true;
	}
    if (action==GLFW_RELEASE) {

		
        if (key== GLFW_KEY_W) isWPressed =false;
        if (key==GLFW_KEY_S) isSPressed = false;
        if (key==GLFW_KEY_A) isAPressed = false;
        if (key==GLFW_KEY_D) isDPressed = false;
		if (key == GLFW_KEY_E) isEPressed = false;
		if (key == GLFW_KEY_Q) isQPressed = false;
    }
}

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}


void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0,0,width,height);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);
	//Wczytanie i import obrazka – w initOpenGLProgram
	
	groundTex = readTexture("tiles.png");
	cubeTex = readTexture("wood.png");
	sandTex = readTexture("sand.png");
	plant1Tex = readTexture("plant1.png");
	rockTex = readTexture("rock.png");
	coralTex = readTexture("coral.png");
	fish1Tex = readTexture("fish1.png");
	fish2Tex = readTexture("fish2.png");

	spcol = new ShaderProgram("v_colored.glsl", NULL, "f_colored.glsl");
	sp=new ShaderProgram("v_simplest.glsl",NULL,"f_simplest.glsl");

}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
	//Usunięcie tekstury z pamięci karty graficznej – w freeOpenGLProgram
	glDeleteTextures(1, &groundTex);
	glDeleteTextures(1, &cubeTex);
	glDeleteTextures(1, &sandTex);
	glDeleteTextures(1, &plant1Tex);
	glDeleteTextures(1, &rockTex);
	glDeleteTextures(1, &coralTex);
	glDeleteTextures(1, &fish1Tex);
	glDeleteTextures(1, &fish2Tex);


    delete sp;
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float angle1,float angle2) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V=glm::lookAt(
    glm::vec3(cameraX, 5.5f, cameraZ),
    glm::vec3(cameraX+cos(cameraAngleY)*20,0, cameraZ +20*sin(cameraAngleY)),
	glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 0.01f, 50.0f); //Wylicz macierz rzutowania

	//podloga ==========================================================================
    glm::mat4 Mground=glm::mat4(1.0f);

	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"),1,false,glm::value_ptr(Mground));

    glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"),4,GL_FLOAT,false,0, groundVertices); 

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, groundVertexNormals); 

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, groundTexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, groundTex);

	glDrawArrays(GL_TRIANGLES,0,groundVertexCount);

    glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));

	//akwarium ====================================================================================
	glm::mat4 M_cube = glm::mat4(1.0f);
	M_cube = glm::scale(M_cube, glm::vec3(10.f, 3.f, 5.f)); //Wylicz macierz modelu

	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_cube));

	glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, myCubeVertexNormals);

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, myCubeTexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, cubeTex);


	glDrawArrays(GL_TRIANGLES,0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));

	glm::mat4 M_cube_top = glm::mat4(1.0f);
	M_cube_top = glm::scale(M_cube_top, glm::vec3(10.f, 3.f, 5.f)); //Wylicz macierz modelu
	M_cube_top = glm::translate(M_cube_top, glm::vec3(0.0f, 4.0f, 0.0f));
	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_cube_top));

	glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, myCubeVertexNormals);

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, myCubeTexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, cubeTex);
	

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));

	//piasek ===================
	glm::mat4 M_sand = glm::mat4(1.0f);
	M_sand = glm::scale(M_sand, glm::vec3(9.9f, 3.1f, 4.9f)); //Wylicz macierz modelu

	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_sand));

	glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, myCubeVertexNormals);

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, myCubeTexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, sandTex);
	

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));

	//rosliny =============
	glm::mat4 M_plant1 = glm::mat4(1.0f);
	M_plant1 = glm::translate(M_plant1, glm::vec3(8.0f, 3.1f, -3.5f));
	M_plant1 = glm::scale(M_plant1, glm::vec3(6.f, 6.f, 6.f)); //Wylicz macierz modelu

	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_plant1));

	glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, plant1Vertices);

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, plant1VertexNormals);

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, plant1TexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, plant1Tex);
	

	glDrawArrays(GL_TRIANGLES, 0, plant1VertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));
	
	//skala ===============================================
	glm::mat4 M_rock = glm::mat4(1.0f);
	M_rock = glm::translate(M_rock, glm::vec3(6.f, 3.1f, -2.f)); //Wylicz macierz modelu
	M_rock = glm::scale(M_rock, glm::vec3(2.f, 2.f, 1.f)); //Wylicz macierz modelu

	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_rock));

	glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, rockVertices);

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, rockVertexNormals);

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, rockTexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, rockTex);
	

	glDrawArrays(GL_TRIANGLES, 0, rockVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));

	// koral ==================================

	glm::mat4 M_coral = glm::mat4(1.0f);
	M_coral = glm::rotate(M_coral, glm::radians(PI), glm::vec3(-1.0f, 0.f, 0.f));

	M_coral = glm::translate(M_coral, glm::vec3(6.f, 3.1f, 2.f)); 

	M_coral = glm::scale(M_coral, glm::vec3(0.05f, 0.05f, 0.05f)); 


	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_coral));

	glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, coralVertices);

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, coralVertexNormals);

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, coralTexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, coralTex);
	

	glDrawArrays(GL_TRIANGLES, 0, coralVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));

	// ryba1 ===============================================
	glm::mat4 M_fish1 = glm::mat4(1.0f);

	M_fish1 = glm::translate(M_fish1, glm::vec3(-3.f, 4.2f, -1.f));
	M_fish1 = glm::scale(M_fish1, glm::vec3(0.5f, 0.5f, 0.5f)); //Wylicz macierz modelu
	M_fish1 = glm::rotate(M_fish1, angle1, glm::vec3(0.0f, 1.0f, 0.0f));
	M_fish1 = glm::translate(M_fish1, glm::vec3(-2.f, 0.f, 0.f));

	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_fish1));

	glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, fish1Vertices);

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, fish1VertexNormals);

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, fish1TexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, fish1Tex);
	

	glDrawArrays(GL_TRIANGLES, 0, fish1VertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));

	// ryba2 ==================================================
	glm::mat4 M_fish2 = glm::mat4(1.0f);

	M_fish2 = glm::translate(M_fish2, glm::vec3(3.f, 5.2f, 1.f));
	M_fish2 = glm::scale(M_fish2, glm::vec3(0.2f, 0.2f, 0.2f)); //Wylicz macierz modelu
	M_fish2 = glm::rotate(M_fish2, angle2, glm::vec3(0.0f, 1.0f, 0.0f));
	M_fish2 = glm::translate(M_fish2, glm::vec3(-5.f, 0.f, 0.f));

	sp->use(); glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_fish2));

	glEnableVertexAttribArray(sp->a("vertex"));  glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, fish2Vertices);

	glEnableVertexAttribArray(sp->a("normal")); glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, fish2VertexNormals);

	glEnableVertexAttribArray(sp->a("texCoord")); glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, fish2TexCoords);

	glUniform1i(sp->u("tex"), 0);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, fish2Tex);
	

	glDrawArrays(GL_TRIANGLES, 0, fish2VertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("texCoord"));

	// szyba =====================================================
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glm::mat4 M_glass = glm::mat4(1.0f);

	M_glass = glm::scale(M_glass, glm::vec3(9.95f, 9.f, 4.95f)); //Wylicz macierz modelu

	spcol->use(); glUniformMatrix4fv(spcol->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(spcol->u("V"), 1, false, glm::value_ptr(V)); glUniformMatrix4fv(spcol->u("M"), 1, false, glm::value_ptr(M_glass));

	glEnableVertexAttribArray(spcol->a("vertex"));  glVertexAttribPointer(spcol->a("vertex"), 4, GL_FLOAT, false, 0, glassVertices);

	glEnableVertexAttribArray(spcol->a("color")); glVertexAttribPointer(spcol->a("color"), 4, GL_FLOAT, false, 0, glassColors);


	glDrawArrays(GL_TRIANGLES, 0, fish2VertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));	glDisableVertexAttribArray(sp->a("normal"));	glDisableVertexAttribArray(sp->a("color"));
	glDisable(GL_BLEND);

    glfwSwapBuffers(window); //Przerzuć tylny bufor na przedni
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	float angle1=0; //Aktualny kąt obrotu obiektu
	float angle2=0; //Aktualny kąt obrotu obiektu


	glfwSetTime(0); //Zeruj timer
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
        angle1+=speed1*glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        angle2+=speed2*glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki

		if (isWPressed) {
			
			cameraZ += cameraSpeed* sin(cameraAngleY) * glfwGetTime();
			cameraX += cameraSpeed* cos(cameraAngleY) * glfwGetTime();
		}
		else if(isSPressed) {
			cameraZ -= cameraSpeed * sin(cameraAngleY) * glfwGetTime();
			cameraX -= cameraSpeed * cos(cameraAngleY) * glfwGetTime();
		}

		if (isAPressed) {
			cameraZ -= cameraSpeed * sin(cameraAngleY + PI/2) * glfwGetTime();
			cameraX -= cameraSpeed * cos(cameraAngleY + PI / 2) * glfwGetTime();
		}
		else if (isDPressed) {
			cameraZ += cameraSpeed * sin(cameraAngleY + PI / 2) * glfwGetTime();
			cameraX += cameraSpeed * cos(cameraAngleY + PI / 2) * glfwGetTime();
		}
		if (isEPressed) {
			cameraAngleY += cameraAngleSpeed * glfwGetTime();
		}
		else if (isQPressed) {
			cameraAngleY -= cameraAngleSpeed * glfwGetTime();
		}

        glfwSetTime(0); //Zeruj timer
		drawScene(window,angle1,angle2); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
