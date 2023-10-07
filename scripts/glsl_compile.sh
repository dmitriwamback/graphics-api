SHADER_FOLDER=$1

VERTEX_SHADER=res/shaders/sources/$SHADER_FOLDER/vMain.vert
FRAGMENT_SHADER=res/shaders/sources/$SHADER_FOLDER/fMain.frag

mkdir res/shaders/compiled/$SHADER_FOLDER

glslc $VERTEX_SHADER -o res/shaders/compiled/$SHADER_FOLDER/vMain.spv
glslc $FRAGMENT_SHADER -o res/shaders/compiled/$SHADER_FOLDER/fMain.spv