/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
//...pode ter mais linhas de código aqui!
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateXLeft=false, rotateXRight=false, rotateYUp=false, rotateYDown=false, rotateZPlus=false, rotateZMinus=false;

// Escala inicial de 1
float scale = 0.6f;
// Incremento da escala
const float scaleFactor = 0.1f;

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -> Grégori Fernandes de Lima - CC", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	// Gerando um buffer simples, com a geometria de um triângulo
	GLuint VAO = setupGeometry();


	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	glm::mat4 modelCubo2 = glm::mat4(1); //matriz identidade do cubo 2

	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	modelCubo2 = glm::rotate(modelCubo2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCubo2)); // cubo 2

	glEnable(GL_DEPTH_TEST);

	// Variáveis para acumular a rotação de cada eixo separadamente
	//   para evitar que ao se modificar o eixo de rotação a rotação anterior seja zerada 
	//     e não perder a rotação dos eixos anteriores
	float rotationX = 0.0f;
	float rotationY = 0.0f;
	float rotationZ = 0.0f;

	float lastTime = glfwGetTime();
	// acumula o ângulo de rotação para evitar que a rotação da imagem não seja baseada em um estado inicial
	float rotationAngle = 0.0f; // ângulo acumulado

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Atualiza os ângulos conforme o input
		float rotationSpeed = glm::radians(90.0f); // 90 graus por segundo


		if (rotateXLeft) {
			rotationX += rotationSpeed * deltaTime; // aumenta no tempo
		}
		if (rotateXRight) {
			rotationX -= rotationSpeed * deltaTime; // diminui no tempo
		}
		if (rotateYUp) {
			rotationY += rotationSpeed * deltaTime; // aumenta no tempo
		}
		if (rotateYDown) {
			rotationY -= rotationSpeed * deltaTime; // aumenta no tempo
		}
		if (rotateZPlus) {
			rotationZ += rotationSpeed * deltaTime; // aumenta no tempo
		}
		if (rotateZMinus) {
			rotationZ -= rotationSpeed * deltaTime; // aumenta no tempo
		}
		
		// Resetar model
		model = glm::mat4(1.0f);
		modelCubo2 = glm::mat4(1.0f);

		model = glm::scale(model, glm::vec3(scale, scale, scale)); // Aplica a escala
		modelCubo2 = glm::scale(modelCubo2, glm::vec3(scale, scale, scale));

		// move o cubo 1 mais para perto da borda
		model = glm::translate(model, glm::vec3(-0.85f, 0.0f, 0.0f));
		// move o cubo2 para o lado para ficar visível
		modelCubo2 = glm::translate(modelCubo2, glm::vec3(0.85f, 0.0f, 0.0f));

		// Aplica as rotações acumuladas
		model = glm::rotate(model, rotationX, glm::vec3(1.0f, 0.0f, 0.0f)); // eixo X
		model = glm::rotate(model, rotationY, glm::vec3(0.0f, 1.0f, 0.0f)); // eixo Y
		model = glm::rotate(model, rotationZ, glm::vec3(0.0f, 0.0f, 1.0f)); // eixo Z

		// rotação cubo 2
		modelCubo2 = glm::rotate(modelCubo2, rotationX, glm::vec3(1.0f, 0.0f, 0.0f)); // eixo X
		modelCubo2 = glm::rotate(modelCubo2, rotationY, glm::vec3(0.0f, 1.0f, 0.0f)); // eixo Y
		modelCubo2 = glm::rotate(modelCubo2, rotationZ, glm::vec3(0.0f, 0.0f, 1.0f)); // eixo Z

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36); // número de triângulos
		glDrawArrays(GL_POINTS, 0, 36); // de pontos
		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES
		
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCubo2));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_POINTS, 0, 36);

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		rotateXLeft = true;
		rotateXRight = false;
		rotateYUp = false;
		rotateYDown = false;
		rotateZPlus = false;
		rotateZMinus = false;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		rotateXLeft = false;
		rotateXRight = true;
		rotateYUp = false;
		rotateYDown = false;
		rotateZPlus = false;
		rotateZMinus = false;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		rotateXLeft = false;
		rotateXRight = false;
		rotateYUp = true;
		rotateYDown = false;
		rotateZPlus = false;
		rotateZMinus = false;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		rotateXLeft = false;
		rotateXRight = false;
		rotateYUp = false;
		rotateYDown = true;
		rotateZPlus = false;
		rotateZMinus = false;
	}

	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		rotateXLeft = false;
		rotateXRight = false;
		rotateYUp = false;
		rotateYDown = false;
		rotateZPlus = true;
		rotateZMinus = false;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
	{
		rotateXLeft = false;
		rotateXRight = false;
		rotateYUp = false;
		rotateYDown = false;
		rotateZPlus = false;
		rotateZMinus = true;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		rotateXLeft = false;
		rotateXRight = false;
		rotateYUp = false;
		rotateYDown = false;
		rotateZPlus = false;
		rotateZMinus = false;
	}

	// Escala
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) // tecla '+'
	{
		scale += scaleFactor; // aumenta a escala
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) // tecla '-'
	{
		scale -= scaleFactor;
		if (scale < scaleFactor) scale = scaleFactor; // previne escala negativa
	}


}

//Esta função está bastante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{

	// base quadrada
	GLfloat pointA[] = {-0.5f, -0.5f, -0.5f};  // Ponto A
	GLfloat pointB[] = {0.5f, -0.5f, -0.5f};  // Ponto B
	GLfloat pointC[] = {-0.5f, -0.5f, 0.5f};  // Ponto C
	GLfloat pointD[] = {0.5f, -0.5f, 0.5f};   // Ponto D

	// topo quadrado
	GLfloat pointE[] = {-0.5f, 0.5f, -0.5f};  // Ponto E
	GLfloat pointF[] = {0.5f, 0.5f, -0.5f};  // Ponto F
	GLfloat pointG[] = {-0.5f, 0.5f, 0.5f};  // Ponto G
	GLfloat pointH[] = {0.5f, 0.5f, 0.5f};   // Ponto H

	GLfloat colorA[] = {1.0f, 0.0f, 0.0f};   // Cor Ponto A
	GLfloat colorB[] = {0.0f, 1.0f, 0.0f};   // Cor Ponto B
	GLfloat colorC[] = {0.0f, 0.0f, 1.0f};   // Cor Ponto C
	GLfloat colorD[] = {1.0f, 1.0f, 0.0f};   // Cor Ponto D
	GLfloat colorE[] = {1.0f, 0.0f, 1.0f};   // Cor Ponto E
	GLfloat colorF[] = {0.0f, 1.0f, 1.0f};   // Cor Ponto F
	GLfloat colorG[] = {1.0f, 0.5f, 0.0f};   // Cor Ponto G
	GLfloat colorH[] = {0.5f, 0.0f, 1.0f};   // Cor Ponto H

	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {

		// total, 12 triângulos

		//Base do quadrado com 2 triângulos:
		//  x         y           z          r          g         b
        pointA[0], pointA[1], pointA[2], colorA[0], colorA[1], colorA[2],
        pointB[0], pointB[1], pointB[2], colorB[0], colorB[1], colorB[2],
        pointC[0], pointC[1], pointC[2], colorC[0], colorC[1], colorC[2],
		//  x         y           z          r          g         b
        pointB[0], pointB[1], pointB[2], colorB[0], colorB[1], colorB[2],
        pointC[0], pointC[1], pointC[2], colorC[0], colorC[1], colorC[2],
        pointD[0], pointD[1], pointD[2], colorD[0], colorD[1], colorD[2],

		// //Topo do quadrado com 2 triângulos:
		// //  x       y           z          r          g         b
        pointE[0], pointE[1], pointE[2], colorE[0], colorE[1], colorE[2],
        pointF[0], pointF[1], pointF[2], colorF[0], colorF[1], colorF[2],
        pointG[0], pointG[1], pointG[2], colorG[0], colorG[1], colorG[2],
		//  x         y           z          r          g         b
        pointF[0], pointF[1], pointF[2], colorF[0], colorF[1], colorF[2],
        pointG[0], pointG[1], pointG[2], colorG[0], colorG[1], colorG[2],
        pointH[0], pointH[1], pointH[2], colorH[0], colorH[1], colorH[2],

		// Lado 1 com  2 triângulos:
		//  x         y           z          r          g         b
        pointC[0], pointC[1], pointC[2], colorC[0], colorC[1], colorC[2],
        pointA[0], pointA[1], pointA[2], colorA[0], colorA[1], colorA[2],
        pointE[0], pointE[1], pointE[2], colorE[0], colorE[1], colorE[2],
		// //  x         y           z          r          g      b
        pointC[0], pointC[1], pointC[2], colorC[0], colorC[1], colorC[2],
        pointE[0], pointE[1], pointE[2], colorE[0], colorE[1], colorE[2],
        pointG[0], pointG[1], pointG[2], colorG[0], colorG[1], colorG[2],

		// Lado 2 com  2 triângulos:
		//  x         y           z          r          g         b
        pointB[0], pointB[1], pointB[2], colorB[0], colorB[1], colorB[2],
        pointF[0], pointF[1], pointF[2], colorF[0], colorF[1], colorF[2],
        pointD[0], pointD[1], pointD[2], colorD[0], colorD[1], colorD[2],
		//  x         y           z          r          g         b
        pointD[0], pointD[1], pointD[2], colorD[0], colorD[1], colorD[2],
        pointF[0], pointF[1], pointF[2], colorF[0], colorF[1], colorF[2],
        pointH[0], pointH[1], pointH[2], colorH[0], colorH[1], colorH[2],

		// Frente com 2 triângulos
		//  x         y           z          r          g         b
		pointD[0], pointD[1], pointD[2], colorD[0], colorD[1], colorD[2],
		pointH[0], pointH[1], pointH[2], colorH[0], colorH[1], colorH[2],
		pointC[0], pointC[1], pointC[2], colorC[0], colorC[1], colorC[2],
		//  x         y           z          r          g         b	
		pointC[0], pointC[1], pointC[2], colorC[0], colorC[1], colorC[2],
		pointH[0], pointH[1], pointH[2], colorH[0], colorH[1], colorH[2],
		pointG[0], pointG[1], pointG[2], colorG[0], colorG[1], colorG[2],

		// Trás com 2 triângulos
		//  x         y           z          r          g         b
		pointB[0], pointB[1], pointB[2], colorB[0], colorB[1], colorB[2],
		pointA[0], pointA[1], pointA[2], colorA[0], colorA[1], colorA[2],
		pointF[0], pointF[1], pointF[2], colorF[0], colorF[1], colorF[2],
		//  x         y           z          r          g         b
		pointF[0], pointF[1], pointF[2], colorF[0], colorF[1], colorF[2],
		pointA[0], pointA[1], pointA[2], colorA[0], colorA[1], colorA[2],
		pointE[0], pointE[1], pointE[2], colorE[0], colorE[1], colorE[2],

	};

	GLuint VBO, VAO;

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

