using namespace std;

struct SymbolInformation {
	char* code;
	char* path;
};

void freeType_init(const string font_name, std::vector<SymbolInformation> symbols_informations);
void RenderText(VSShaderLib& shader, string text, bool center, float x, float y, float scale, float cR, float cG, float cB);