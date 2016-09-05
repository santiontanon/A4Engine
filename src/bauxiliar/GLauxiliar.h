#ifdef __APPLE__
    #define glGenVertexArrays glGenVertexArraysAPPLE
    #define glBindVertexArray glBindVertexArrayAPPLE
#endif

extern GLuint programID;

extern GLuint PVMatrixID;
extern GLuint MMatrixID;
extern GLuint inColorID;
extern GLuint useTextureID;

GLuint LoadShaders();

void setTexture(GLuint tex);    // This function keeps track of the previous texture being used,
// if the new one is the same as before, it saves the cost of loading it again.
GLuint  createTexture(SDL_Surface *sfc,float *tx,float *ty);

// Note: this function is VERY slow, since it creates and destroys a new vertex buffer each time it's called:
void clearDrawQuadData();
void drawQuad(float x, float y, float w, float h, float r, float g, float b, float a);
