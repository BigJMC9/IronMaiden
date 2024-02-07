#pragma once

#include "H_JCVB_component.hpp"
#include "H_JCVB_model.hpp"
#include "H_JCVB_material.hpp"

//std
#include <memory>
#include <unordered_map>
#include <iostream>

namespace Digestion {
	class JcvbMeshRenderer : public JcvbComponent{
	public:
		JcvbMeshRenderer(std::shared_ptr<Model> jcvbModel, std::shared_ptr<Material> jcvbMaterial);
		JcvbMeshRenderer(std::shared_ptr<Model> jcvbModel);
		~JcvbMeshRenderer();

		

		std::shared_ptr<Model> getModel(){
			return model;
		}

		std::shared_ptr<Material> getMaterial(){
			return material;
		}

		void start() override;
		void update() override;
		void render() override;

	private:
		std::shared_ptr<Model> model;
		std::shared_ptr<Material> material;
		bool firstFrame;
	};
}