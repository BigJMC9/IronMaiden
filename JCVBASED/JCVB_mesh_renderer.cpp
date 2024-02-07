#include "H_JCVB_mesh_renderer.hpp"
#include "H_JCVB_game_object.hpp"
//std
#include <iostream>

namespace Digestion {
	JcvbMeshRenderer::JcvbMeshRenderer(std::shared_ptr<Model> jcvbModel, std::shared_ptr<Material> jcvbMaterial) : model(jcvbModel), material(jcvbMaterial), firstFrame(true) { }
	JcvbMeshRenderer::JcvbMeshRenderer(std::shared_ptr<Model> jcvbModel) : model(jcvbModel), material(nullptr), firstFrame(true) { }
	JcvbMeshRenderer::~JcvbMeshRenderer() {}

	void JcvbMeshRenderer::start(){
		std::cout << "start \n";
	}
	void JcvbMeshRenderer::update(){
		//std::cout << "update \n";
	}
	void JcvbMeshRenderer::render(){
		if (firstFrame) {
			std::cout << "Render: " << gameObject->name << std::endl;
		}
		firstFrame = false;
	}
}