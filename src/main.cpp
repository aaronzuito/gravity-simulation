#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char *vertexShaderSource = "#version 460 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
"}\0";

const char *fragmentShaderSource = "#version 460 core\n"
"out vec4 FragColor;\n"
"void main(){\n"
"FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\0";

const double G = 6.6743e-11f; // Newton's gravitational constant
GLFWwindow* StartGLFW();


void initShader();
void manageInput(GLFWwindow* window);
void resizeWindowView(GLFWwindow* window, int width, int heigth);
GLuint shaderProgram;

class entity{
	public:
		int vertexCount;
		unsigned int VAO, VBO;
	
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		double mass;
		float ratio;
		float density;
		glm::vec4 color;
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		
		std::vector<float> createVertices(float ratio, long int res){
			std::vector<float> vertices;
	
			vertices.push_back(0.0f), vertices.push_back(0.0f), vertices.push_back(0.0f);

			for (int i=0; i<=res; i++){
				float angle = (i * 2.0f * 3.1415926535f) / res; 
				vertices.push_back(std::cos(angle) * ratio); //x
				vertices.push_back(std::sin(angle) * ratio); //y
				vertices.push_back(0.0f); //z

			}	
			vertexCount = vertices.size() / 3.0f;
			return vertices;
		}	

		entity(glm::vec3 initPosition, glm::vec3 initVelocity, double mass, float ratio, glm::vec4 color){

			position = initPosition;
			velocity = initVelocity;
			this -> mass = mass;
			this -> ratio = ratio;
			this -> density = mass / ((4.0f/3.0f)*3.1415926535f*std::pow(ratio, 3.0f));
			this -> color = color;
			
			std::vector<float> v = createVertices(ratio, 48);
			creatingVBOVAO(v);
		}

	void creatingVBOVAO(const std::vector<float> vertices){
	
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
	
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
}

	void render(unsigned int shaderID){
		glUseProgram(shaderID);	
		// model matrix
		int modelLoc = glGetUniformLocation(shaderID, "model");
		glUniformMatrix4fv(modelLoc,1 , GL_FALSE, glm::value_ptr(model));

		// view matrix	
		int viewLoc = glGetUniformLocation(shaderID, "view");
		glUniformMatrix4fv(viewLoc,1 , GL_FALSE, glm::value_ptr(view));

		// projection matrix
		int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionLoc,1 , GL_FALSE, glm::value_ptr(projection));

 
	// outside render loop
	
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
		glBindVertexArray(0);

	}

		 
	
};
void checkWallColisions(entity &object);
void checkColisions(entity &object1, entity &object2);

	
int main(){

	GLFWwindow* window = StartGLFW();
	
	if (!window){

		std::cerr << "Failed to create GLFW window" << std::endl;
		return 1;
	}
	
	glfwSetFramebufferSizeCallback(window, resizeWindowView);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	initShader();
	
		
	std::vector<entity> objects = {
	entity(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 5.94e24f, 0.2f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
	entity(glm::vec3(1.0f,0.0f,0.0f), glm::vec3(0.0f, 0.02f, 0.0f), 7.34e22f, 0.1f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
	};
	
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);	

	while (!glfwWindowShouldClose(window)){						
	
	manageInput(window);
	glClear(GL_COLOR_BUFFER_BIT);	
	
	for(auto &object : objects){
		float acceleration;
		glm::vec3 direction;
		
		for(auto &object2 : objects){
			if(&object == &object2){continue;}
			
			double distance = glm::distance(object2.position, object.position);
			distance *= 10e8;
			direction = glm::normalize(object.position - object2.position);

			float Gforce = -(G*object.mass*object2.mass) / (distance*distance);
			acceleration = (Gforce / object.mass);
			checkColisions(object, object2);

		}
	
		object.velocity += acceleration*direction;
		object.position += object.velocity;
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, object.position);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f,0.0f,0.0f));


		object.model = model;
		object.view = view;
		object.projection = projection;

		object.render(shaderProgram);
	}
	

	//check and call events
	glfwSwapBuffers(window);
	glfwPollEvents();
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;


}


GLFWwindow* StartGLFW(){		 
	
	if(!glfwInit()){

		std::cerr << "Failed to initialize the window" << std::endl;
		glfwTerminate();
		return nullptr;	
	}
	
	GLFWwindow* window = glfwCreateWindow(800, 800, "simulation", NULL, NULL); // function that creates the window
	
	if (!window){

		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}
	
	return window;
}

// Input management

void manageInput(GLFWwindow* window){

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		
		glfwSetWindowShouldClose(window, true);
	} 

}

// Window resize managment

void resizeWindowView(GLFWwindow* window, int width, int heigth){
	glViewport(0, 0, width, heigth);
}

// Shader


void initShader(){
	int success;
	char infoLog[512];

	//vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if(!success){
	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	std::cout << "Error compiling  vertex shader: \n" << infoLog << std::endl;
	}

	//fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	
	if(!success){
	glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
	std::cout << "Error compiling fragment shader: \n" << infoLog << std::endl;
	}

	//shaderProgram
	
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);	
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	
	if(!success) {
	glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	}

	glUseProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


}

// checking colisions 		
void checkWallColisions(entity &object){
	
	// walls
	if (std::abs(object.position.y) + object.ratio >= 1.0f){
		if (object.position.y < 0.0f){
		object.position.y = -1.0f + object.ratio;
		object.velocity.y *= -0.75f;
		}
		else {
		object.position.y = 1.0f - object.ratio;
		object.velocity.y *= -0.75f;
		}
	}

	if (std::abs(object.position.x) + object.ratio >= 1.0f){
		if (object.position.x < 0.0f){
		object.position.x = -1.0f + object.ratio;
		object.velocity.x *= -0.75f;
		}
		else {
		object.position.x = 1.0f - object.ratio;
		object.velocity.x *= -0.75f;
		}
	}
	
}

void checkColisions(entity &object1, entity &object2){
	
	float dx = object2.position.x - object1.position.x;
	float dy = object2.position.y - object1.position.y;



	float squaredCenterDistance = dx*dx + dy*dy; 
	float squaredRadiusSum = (object1.ratio + object2.ratio)*(object1.ratio + object2.ratio);
	
	if(squaredCenterDistance <= squaredRadiusSum){
		glm::vec3 normal = glm::normalize(object2.position - object1.position);
		glm::vec3 tangent = glm::vec3(-normal.y, normal.x, 0.0f);
		
		glm::vec3 v1 = object1.velocity;
		glm::vec3 v2 = object2.velocity;	



		float v1n = glm::dot(normal, v1);
		float v1t = glm::dot(tangent, v1);
		float v2n = glm::dot(normal, v2);
		float v2t = glm::dot(tangent, v2);

		float v1nF = (v1n*(object1.mass - object2.mass) + 2.0f*object2.mass*v2n) / (object1.mass + object2.mass);
		float v2nF =  (v2n*(object2.mass - object1.mass) + 2.0f*object1.mass*v1n) / (object1.mass + object2.mass);

		object1.velocity = (v1nF*normal) + (v1t*tangent);
		object2.velocity = (v2nF*normal) + (v2t*tangent);
	
		float overlap = (object1.ratio + object2.ratio) - glm::distance(object1.position, object2.position);
		if (overlap > 0){
		object1.position -= normal*(overlap / 2.0f);
		object2.position += normal*(overlap / 2.0f);


		}

	}
}

