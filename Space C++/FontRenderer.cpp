#include "FontRenderer.h"
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace FontRenderer {
#ifdef _WIN32
	static GLuint fontBase = 0;
	static HDC hDC = nullptr;
#endif
	void initFont() {
#ifdef _WIN32
		fontBase = glGenLists(256);

		hDC = wglGetCurrentDC();

		if (hDC) {
			HFONT hFont = CreateFont(
				-16, // Height
				0, // Width
				0, 0, // Escapement, Orientation
				FW_NORMAL, // Weight
				FALSE, FALSE, FALSE, // Italic, Underline, StrikeOut
				ANSI_CHARSET,
				OUT_TT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY,
				FF_MODERN | FIXED_PITCH,
				TEXT("Consolas") // Font name
			);

			if (hFont) {
				HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
				wglUseFontBitmaps(hDC, 0, 256, fontBase);
				SelectObject(hDC, hOldFont);
				DeleteObject(hFont);
			}
		}
#endif
	}

	void renderText(const std::string& text, float x, float y, float scale,
		float r, float g, float b, float a) {
#ifdef _WIN32
		if (fontBase == 0 || text.empty()) return;

		glColor4f(r, g, b, a);

		glRasterPos2f(x, y);

		glPushAttrib(GL_LIST_BIT);
		glListBase(fontBase);

		// Draw the string
		glCallLists((GLsizei)text.length(), GL_UNSIGNED_BYTE, text.c_str());

		glPopAttrib();
#endif
	}

	float getTextWidth(const std::string& text, float scale) {
#ifdef _WIN32
		if (hDC && !text.empty()) {
			SIZE size;
			GetTextExtentPoint32A(hDC, text.c_str(), (int)text.length(), &size);
			return (float)size.cx * scale;
		}
#endif
		return text.length() * 8.0f * scale;
	}

	void cleanup() {
#ifdef _WIN32
		if (fontBase != 0) {
			glDeleteLists(fontBase, 256);
			fontBase = 0;
		}
#endif
	}
}
