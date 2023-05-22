#include "raylib"

class KeyVisualizer {
private:
	char keys[256];
	
public:
	void setKeys(char *keys) {
		memcpy(this.keys, keys, 256);
	}

}