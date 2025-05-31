#include <iostream>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();
GLuint loadTexture(string filePath, int &width, int &height);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 600, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"layout (location = 2) in vec3 normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec2 TexCoord;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"void main()\n"
"{\n"
"    gl_Position = projection * view * model * vec4(position, 1.0);\n"
"    TexCoord = texCoord;\n"
"    FragPos = vec3(model * vec4(position, 1.0));\n"
"    Normal = mat3(transpose(inverse(model))) * normal;\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec2 TexCoord;\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"out vec4 FragColor;\n"
"uniform sampler2D texBuff;\n"
"uniform float ka;\n"
"uniform float kd;\n"
"uniform float ks;\n"
"uniform float shininess;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 lightColor;\n"
"uniform vec3 viewPos;\n"
"void main()\n"
"{\n"
"    // 1. Iluminação ambiente\n"
"    vec3 ambient = ka * lightColor;\n"
"    \n"
"    // 2. Iluminação difusa\n"
"    vec3 norm = normalize(Normal);\n"
"    vec3 lightDir = normalize(lightPos - FragPos);\n"
"    float diff = max(dot(norm, lightDir), 0.0);\n"
"    vec3 diffuse = kd * diff * lightColor;\n"
"    \n"
"    // 3. Iluminação especular (Phong)\n"
"    vec3 viewDir = normalize(viewPos - FragPos);\n"
"    vec3 reflectDir = reflect(-lightDir, norm);\n"
"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
"    vec3 specular = ks * spec * lightColor;\n"
"    \n"
"    // 4. Combinação final (apenas com textura)\n"
"    vec3 texColor = texture(texBuff, TexCoord).rgb;\n"
"    vec3 result = (ambient + diffuse + specular) * texColor;\n"
"    FragColor = vec4(result, 1.0);\n"
"}\n\0";

bool rotateXLeft=false, rotateXRight=false, rotateYUp=false, rotateYDown=false, rotateZPlus=false, rotateZMinus=false;

// Escala inicial de 1
float escala = 0.6f;
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

	// Carregando uma textura e armazenando seu id
	int imgWidth, imgHeight;
	GLuint texID = loadTexture("../assets/tex/pixelWall.png",imgWidth,imgHeight);

	glUseProgram(shaderID);

	// Enviar a informação de qual variável armazenará o buffer da textura
	glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);

	//Ativando o primeiro buffer de textura da OpenGL
	glActiveTexture(GL_TEXTURE0);

	glm::mat4 model = glm::mat4(1); //matriz identidade;

	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);

	// Variáveis para acumular a rotação de cada eixo separadamente
	//   para evitar que ao se modificar o eixo de rotação a rotação anterior seja zerada 
	//     e não perder a rotação dos eixos anteriores
	float rotationX = 0.0f;
	float rotationY = 0.0f;
	float rotationZ = 0.0f;

	float lastTime = glfwGetTime();
	// acumula o ângulo de rotação para evitar que a rotação da imagem não seja baseada em um estado inicial


	// Configurações de iluminação
	glm::vec3 lightPos(0.0f, 0.5f, 1.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // Luz branca
    glm::vec3 viewPos(0.0f, 0.0f, 3.0f);   // Posição da câmera
    
	float ka = 0.3f;    // ambiente
	float kd = 0.8f;    // difusa
	float ks = 1.2f;    // especular
	float shininess = 96.0f;
    
    // Matrizes de view e projection
    glm::mat4 view = glm::lookAt(viewPos, 
                                glm::vec3(0.0f, 0.0f, 0.0f), 
                                glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                          (float)WIDTH / (float)HEIGHT, 
                                          0.1f, 100.0f);


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
		
		// Envia as variáveis de iluminação para o shader
        glUniform3fv(glGetUniformLocation(shaderID, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderID, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(shaderID, "viewPos"), 1, glm::value_ptr(viewPos));
        
        glUniform1f(glGetUniformLocation(shaderID, "ka"), ka);
        glUniform1f(glGetUniformLocation(shaderID, "kd"), kd);
        glUniform1f(glGetUniformLocation(shaderID, "ks"), ks);
        glUniform1f(glGetUniformLocation(shaderID, "shininess"), shininess);
        
        // Envia as matrizes view e projection
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Resetar model
		model = glm::mat4(1.0f);

		model = glm::scale(model, glm::vec3(escala, escala, escala)); // Aplica a escala

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

		// Aplica as rotações acumuladas
		model = glm::rotate(model, rotationX, glm::vec3(1.0f, 0.0f, 0.0f)); // eixo X
		model = glm::rotate(model, rotationY, glm::vec3(0.0f, 1.0f, 0.0f)); // eixo Y
		model = glm::rotate(model, rotationZ, glm::vec3(0.0f, 0.0f, 1.0f)); // eixo Z

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);

		glBindTexture(GL_TEXTURE_2D, texID);
		
		glDrawArrays(GL_TRIANGLES, 0, 36); // número de triângulos
		glDrawArrays(GL_POINTS, 0, 36); // de pontos
		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES

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
		escala += scaleFactor; // aumenta a escala
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) // tecla '-'
	{
		escala -= scaleFactor;
		if (escala < scaleFactor) escala = scaleFactor; // previne escala negativa
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

    // Normais para cada face do cubo
    glm::vec3 normals[] = {
        glm::vec3(0.0f, -1.0f, 0.0f),  // Base
        glm::vec3(0.0f, 1.0f, 0.0f),   // Topo
        glm::vec3(-1.0f, 0.0f, 0.0f),  // Lado esquerdo
        glm::vec3(1.0f, 0.0f, 0.0f),   // Lado direito
        glm::vec3(0.0f, 0.0f, 1.0f),   // Frente
        glm::vec3(0.0f, 0.0f, -1.0f)   // Trás
    };

    GLfloat vertices[] = {
        // Base do quadrado (face inferior)
        pointA[0], pointA[1], pointA[2], 0.0f, 0.0f, normals[0].x, normals[0].y, normals[0].z,
        pointB[0], pointB[1], pointB[2], 1.0f, 0.0f, normals[0].x, normals[0].y, normals[0].z,
        pointC[0], pointC[1], pointC[2], 0.0f, 1.0f, normals[0].x, normals[0].y, normals[0].z,
        
        pointB[0], pointB[1], pointB[2], 1.0f, 0.0f, normals[0].x, normals[0].y, normals[0].z,
        pointC[0], pointC[1], pointC[2], 0.0f, 1.0f, normals[0].x, normals[0].y, normals[0].z,
        pointD[0], pointD[1], pointD[2], 1.0f, 1.0f, normals[0].x, normals[0].y, normals[0].z,

        // Topo do quadrado (face superior)
        pointE[0], pointE[1], pointE[2], 0.0f, 0.0f, normals[1].x, normals[1].y, normals[1].z,
        pointF[0], pointF[1], pointF[2], 1.0f, 0.0f, normals[1].x, normals[1].y, normals[1].z,
        pointG[0], pointG[1], pointG[2], 0.0f, 1.0f, normals[1].x, normals[1].y, normals[1].z,
        
        pointF[0], pointF[1], pointF[2], 1.0f, 0.0f, normals[1].x, normals[1].y, normals[1].z,
        pointG[0], pointG[1], pointG[2], 0.0f, 1.0f, normals[1].x, normals[1].y, normals[1].z,
        pointH[0], pointH[1], pointH[2], 1.0f, 1.0f, normals[1].x, normals[1].y, normals[1].z,

        // Lado esquerdo
        pointA[0], pointA[1], pointA[2], 0.0f, 0.0f, normals[2].x, normals[2].y, normals[2].z,
        pointC[0], pointC[1], pointC[2], 1.0f, 0.0f, normals[2].x, normals[2].y, normals[2].z,
        pointE[0], pointE[1], pointE[2], 0.0f, 1.0f, normals[2].x, normals[2].y, normals[2].z,
        
        pointC[0], pointC[1], pointC[2], 1.0f, 0.0f, normals[2].x, normals[2].y, normals[2].z,
        pointE[0], pointE[1], pointE[2], 0.0f, 1.0f, normals[2].x, normals[2].y, normals[2].z,
        pointG[0], pointG[1], pointG[2], 1.0f, 1.0f, normals[2].x, normals[2].y, normals[2].z,

        // Lado direito
        pointB[0], pointB[1], pointB[2], 0.0f, 0.0f, normals[3].x, normals[3].y, normals[3].z,
        pointD[0], pointD[1], pointD[2], 1.0f, 0.0f, normals[3].x, normals[3].y, normals[3].z,
        pointF[0], pointF[1], pointF[2], 0.0f, 1.0f, normals[3].x, normals[3].y, normals[3].z,
        
        pointD[0], pointD[1], pointD[2], 1.0f, 0.0f, normals[3].x, normals[3].y, normals[3].z,
        pointF[0], pointF[1], pointF[2], 0.0f, 1.0f, normals[3].x, normals[3].y, normals[3].z,
        pointH[0], pointH[1], pointH[2], 1.0f, 1.0f, normals[3].x, normals[3].y, normals[3].z,

        // Frente
        pointC[0], pointC[1], pointC[2], 0.0f, 0.0f, normals[4].x, normals[4].y, normals[4].z,
        pointD[0], pointD[1], pointD[2], 1.0f, 0.0f, normals[4].x, normals[4].y, normals[4].z,
        pointG[0], pointG[1], pointG[2], 0.0f, 1.0f, normals[4].x, normals[4].y, normals[4].z,
        
        pointD[0], pointD[1], pointD[2], 1.0f, 0.0f, normals[4].x, normals[4].y, normals[4].z,
        pointG[0], pointG[1], pointG[2], 0.0f, 1.0f, normals[4].x, normals[4].y, normals[4].z,
        pointH[0], pointH[1], pointH[2], 1.0f, 1.0f, normals[4].x, normals[4].y, normals[4].z,

        // Trás
        pointA[0], pointA[1], pointA[2], 0.0f, 0.0f, normals[5].x, normals[5].y, normals[5].z,
        pointB[0], pointB[1], pointB[2], 1.0f, 0.0f, normals[5].x, normals[5].y, normals[5].z,
        pointE[0], pointE[1], pointE[2], 0.0f, 1.0f, normals[5].x, normals[5].y, normals[5].z,
        
        pointB[0], pointB[1], pointB[2], 1.0f, 0.0f, normals[5].x, normals[5].y, normals[5].z,
        pointE[0], pointE[1], pointE[2], 0.0f, 1.0f, normals[5].x, normals[5].y, normals[5].z,
        pointF[0], pointF[1], pointF[2], 1.0f, 1.0f, normals[5].x, normals[5].y, normals[5].z
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Configura os atributos dos vértices:
    // - Posição (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // - Coordenadas de textura (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // - Normal (location = 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

GLuint loadTexture(string filePath, int &width, int &height)
{
	GLuint texID; // id da textura a ser carregada

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// Ajuste dos parâmetros de wrapping e filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregamento da imagem usando a função stbi_load da biblioteca stb_image
	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // assume que é 4 canais png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture " << filePath << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}