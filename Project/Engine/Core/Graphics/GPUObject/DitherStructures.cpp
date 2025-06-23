#include "DitherStructures.h"

//============================================================================
//	incldue
//============================================================================

// imgui
#include <imgui.h>

//============================================================================
//	DitherStructures classMethods
//============================================================================

void DitherForGPU::Init() {

	maxValue = 64.0f;
	fadeStart = 7.2f;
	fadeLength = 48.0f;
}

void DitherForGPU::ImGui() {

	ImGui::DragFloat("maxValue", &maxValue, 0.1f);
	ImGui::DragFloat("fadeStart", &fadeStart, 0.1f);
	ImGui::DragFloat("fadeLength", &fadeLength, 0.1f);
}