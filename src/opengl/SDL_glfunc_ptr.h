#define GL_FUNC_POINTERS \
	void (*glGetShaderiv)      (GLuint, GLenum, int* );       \
	void (*glGetProgramiv)     (GLuint, GLenum, GLint );      \
	GLuint (*glCreateShader)   (GLenum);                      \
	void (*glShaderSource)     (GLuint, GLsizei,              \
		const GLchar**, const GLint* );                       \
	void (*glCompileShader)    (GLuinti);                     \

