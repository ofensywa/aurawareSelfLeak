#include "../MainInclude.hpp"
#include "NightMode.hpp"
bool executed = false;

ConVar * sv_skyname = nullptr;
std::string fallback_skybox = "";

std::vector<MaterialBackup> materials;
std::vector<MaterialBackup> skyboxes;

void nightmode::clear_stored_materials() {
	fallback_skybox = "";
	sv_skyname = nullptr;
	materials.clear();
	skyboxes.clear();
}

void nightmode::modulate(MaterialHandle_t i, IMaterial *material, bool backup = false) {
	if (strstr(material->GetTextureGroupName(), "World")) {
		if (backup) materials.push_back(MaterialBackup(i, material));

		material->ColorModulate(0.15f, 0.15f, 0.15f);
	}
	else if (strstr(material->GetTextureGroupName(), "StaticProp")) {
		if (backup)
			materials.push_back(MaterialBackup(i, material));

		material->ColorModulate(0.4f, 0.4f, 0.4f);
	}
}

void nightmode::apply() {
	if (executed || !g_CVar->Init()) {
		return;
	}

	executed = true;

	if (!sv_skyname) {
		sv_skyname = g_CVar->FindVar("sv_skyname");
		sv_skyname->m_nFlags &= ~FCVAR_CHEAT;
	}

	fallback_skybox = sv_skyname->GetString();
	sv_skyname->SetValue("sky_csgo_night02");

	g_CVar->FindVar("r_drawspecificstaticprop")->SetValue(0);

	if (materials.size()) {
		for (int i = 0; i < (int)materials.size(); i++)
			modulate(materials[i].handle, materials[i].material);

		return;
	}

	materials.clear();

	for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i)) {
		IMaterial* material = g_MatSystem->GetMaterial(i);

		if (!material || material->IsErrorMaterial()) {
			continue;
		}

		if (material->GetReferenceCount() <= 0) {
			continue;
		}

		modulate(i, material, true);
	}
}

void nightmode::remove() {
	if (!executed || !g_CVar->Init()) {
		return;
	}

	executed = false;
	g_Options.NightMode = false;

	if (sv_skyname) {
		sv_skyname->SetValue(fallback_skybox.c_str());
	}

	g_CVar->FindVar("r_drawspecificstaticprop")->SetValue(1);

	for (int i = 0; i < materials.size(); i++) {
		if (materials[i].material->GetReferenceCount() <= 0) continue;

		materials[i].restore();
		materials[i].material->Refresh();
	}

	materials.clear();

	sv_skyname = nullptr;
	fallback_skybox = "";
}