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

#include <vector>
#include <glm/glm.hpp>

struct Trajectory {
    std::vector<glm::vec3> controlPoints;
    float currentTime = 0.0f;
    float speed = 0.03f;  
    bool isActive = false;
};

std::vector<Trajectory> objectTrajectories;
int selectedObjectIndex = 0;

void addControlPoint(glm::vec3 point);

glm::vec3 calculatePosition(Trajectory& trajectory);

void toggleTrajectory();

void clearTrajectory();

void initDefaultTrajectory();

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
	objectTrajectories.push_back(Trajectory());

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
    
	objectTrajectories.push_back(Trajectory());

    initDefaultTrajectory();

    // Matrizes de view e projection
   glm::mat4 view = glm::lookAt(
		glm::vec3(2.0f, 2.0f, 3.0f),  // Posição da câmera (mais afastada em Z)
		glm::vec3(0.0f, 0.0f, 0.0f),  // Ponto para onde olha
		glm::vec3(0.0f, 1.0f, 0.0f)   // Vetor "para cima"
	);
    glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),
		(float)WIDTH / (float)HEIGHT,
		0.1f,
		100.0f
	);


	// Loop da aplicação - "game loop"
	// Loop da aplicação - "game loop"
while (!glfwWindowShouldClose(window))
{
    // Checa eventos de input
    glfwPollEvents();

    // Limpa os buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Atualiza o tempo
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Atualiza rotações baseadas no input
    float rotationSpeed = glm::radians(90.0f) * deltaTime;
    if (rotateXLeft) rotationX += rotationSpeed;
    if (rotateXRight) rotationX -= rotationSpeed;
    if (rotateYUp) rotationY += rotationSpeed;
    if (rotateYDown) rotationY -= rotationSpeed;
    if (rotateZPlus) rotationZ += rotationSpeed;
    if (rotateZMinus) rotationZ -= rotationSpeed;

    // Reset da matriz model (APENAS UMA VEZ por frame)
    model = glm::mat4(1.0f);

    // 1. Aplica escala
    model = glm::scale(model, glm::vec3(escala, escala, escala));

    // 2. Aplica trajetória (se ativa e com pontos suficientes)
    if (objectTrajectories.size() > selectedObjectIndex && 
        objectTrajectories[selectedObjectIndex].isActive && 
        objectTrajectories[selectedObjectIndex].controlPoints.size() >= 2) 
    {
        glm::vec3 trajectoryPos = calculatePosition(objectTrajectories[selectedObjectIndex]);
        model = glm::translate(model, trajectoryPos);
    }

    // 3. Aplica rotações
    model = glm::rotate(model, rotationX, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotationZ, glm::vec3(0.0f, 0.0f, 1.0f));

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

    // Envia a matriz model atualizada
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Renderiza o objeto
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, texID);
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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Controles para trajetória
   // Controles para trajetória
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        toggleTrajectory();
        std::cout << "Trajectory " << (objectTrajectories[selectedObjectIndex].isActive ? "activated" : "deactivated") << std::endl;
    }
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        clearTrajectory();
        std::cout << "Trajectory cleared" << std::endl;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        static int pointCount = 0;
        glm::vec3 newPoint;
        switch(pointCount % 8) {
            case 0: newPoint = glm::vec3(1.0f, 0.0f, 0.0f); break;   // X
            case 1: newPoint = glm::vec3(0.0f, 1.0f, 0.0f); break;   // Y
            case 2: newPoint = glm::vec3(-1.0f, 0.0f, 0.0f); break;  // -X
            case 3: newPoint = glm::vec3(0.0f, -1.0f, 0.0f); break;  // -Y
            case 4: newPoint = glm::vec3(0.0f, 0.0f, 1.0f); break;   // Z
            case 5: newPoint = glm::vec3(1.0f, 1.0f, 0.0f); break;   // XY
            case 6: newPoint = glm::vec3(0.0f, 1.0f, 1.0f); break;   // YZ
            case 7: newPoint = glm::vec3(1.0f, 0.0f, 1.0f); break;   // XZ
        }
        addControlPoint(newPoint);
        pointCount++;
        std::cout << "Added control point at (" 
                 << newPoint.x << ", " 
                 << newPoint.y << ", " 
                 << newPoint.z << ")" << std::endl;
    }

    // Ajuste mais fino da velocidade
    if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
        if (objectTrajectories.size() > selectedObjectIndex) {
            objectTrajectories[selectedObjectIndex].speed += 0.05f;  // Incremento menor
            std::cout << "Speed increased to: " << objectTrajectories[selectedObjectIndex].speed << std::endl;
        }
    }
    if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
        if (objectTrajectories.size() > selectedObjectIndex) {
            objectTrajectories[selectedObjectIndex].speed = std::max(0.01f,  // Velocidade mínima muito menor
                objectTrajectories[selectedObjectIndex].speed - 0.05f);  // Decremento menor
            std::cout << "Speed decreased to: " << objectTrajectories[selectedObjectIndex].speed << std::endl;
        }
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

void addControlPoint(glm::vec3 point) {
    if (objectTrajectories.size() <= selectedObjectIndex) {
        objectTrajectories.resize(selectedObjectIndex + 1);
    }
    objectTrajectories[selectedObjectIndex].controlPoints.push_back(point);
}

void toggleTrajectory() {
    if (objectTrajectories.size() > selectedObjectIndex) {
        objectTrajectories[selectedObjectIndex].isActive = !objectTrajectories[selectedObjectIndex].isActive;
    }
}

void clearTrajectory() {
    if (objectTrajectories.size() > selectedObjectIndex) {
        objectTrajectories[selectedObjectIndex].controlPoints.clear();
        objectTrajectories[selectedObjectIndex].isActive = false;
    }
}

glm::vec3 calculatePosition(Trajectory& trajectory) {
    if (trajectory.controlPoints.size() < 2) {
        return glm::vec3(0.0f);
    }
    
    // Incremento de tempo mais preciso e controlável
    trajectory.currentTime += 0.005f * trajectory.speed;
    if (trajectory.currentTime >= 1.0f) {
        trajectory.currentTime = 0.0f;
    }
    
    float t = trajectory.currentTime * (trajectory.controlPoints.size() - 1);
    int segment = static_cast<int>(t);
    t = t - segment;
    
    if (segment >= trajectory.controlPoints.size() - 1) {
        segment = trajectory.controlPoints.size() - 2;
        t = 1.0f;
    }
    
    // Interpolação linear entre os pontos
    return glm::mix(trajectory.controlPoints[segment], 
                   trajectory.controlPoints[segment + 1], 
                   t);
}

void initDefaultTrajectory() {
    if (objectTrajectories.empty()) {
        objectTrajectories.push_back(Trajectory());
    }
    
    // Limpa qualquer trajetória existente
    objectTrajectories[selectedObjectIndex].controlPoints.clear();
    
    // Define os pontos da trajetória (cubo 3D)
    float range = 1.1f; // Alcance do movimento
    objectTrajectories[selectedObjectIndex].controlPoints = {
        glm::vec3( range,  range,  range),  // Ponto 1: frente, cima, direita
        glm::vec3(-range,  range,  range),  // Ponto 2: frente, cima, esquerda
        glm::vec3(-range, -range,  range),  // Ponto 3: frente, baixo, esquerda
        glm::vec3( range, -range,  range),  // Ponto 4: frente, baixo, direita
        glm::vec3( range, -range, -range),  // Ponto 5: trás, baixo, direita
        glm::vec3( range,  range, -range),  // Ponto 6: trás, cima, direita
        glm::vec3(-range,  range, -range),  // Ponto 7: trás, cima, esquerda
        glm::vec3(-range, -range, -range),  // Ponto 8: trás, baixo, esquerda
        glm::vec3(-range, -range,  range),  // Volta para ponto 3
        glm::vec3(-range,  range,  range),  // Volta para ponto 2
        glm::vec3( range,  range,  range)   // Volta para ponto 1 (fecha o ciclo)
    };
    
    // Configurações da trajetória
    objectTrajectories[selectedObjectIndex].speed = 0.015f;
    objectTrajectories[selectedObjectIndex].isActive = true;
    
    std::cout << "Trajetória 3D padrão criada com " 
              << objectTrajectories[selectedObjectIndex].controlPoints.size() 
              << " pontos de controle." << std::endl;
}