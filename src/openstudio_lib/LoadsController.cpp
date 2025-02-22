/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) 2020-2022, OpenStudio Coalition and other contributors. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
*  disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote products
*  derived from this software without specific prior written permission from the respective party.
*
*  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative works
*  may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without specific prior
*  written permission from Alliance for Sustainable Energy, LLC.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND ANY CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S), ANY CONTRIBUTORS, THE UNITED STATES GOVERNMENT, OR THE UNITED
*  STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
*  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************************************************/

#include "LoadsController.hpp"
#include "LoadsView.hpp"

#include <openstudio/model/SpaceLoadDefinition.hpp>
#include <openstudio/model/SpaceLoadDefinition_Impl.hpp>
#include <openstudio/model/SpaceLoadInstance.hpp>
#include <openstudio/model/InternalMassDefinition.hpp>
#include <openstudio/model/InternalMassDefinition_Impl.hpp>
#include <openstudio/model/LightsDefinition.hpp>
#include <openstudio/model/LightsDefinition_Impl.hpp>
#include <openstudio/model/LuminaireDefinition.hpp>
#include <openstudio/model/LuminaireDefinition_Impl.hpp>
#include <openstudio/model/ElectricEquipmentDefinition.hpp>
#include <openstudio/model/ElectricEquipmentDefinition_Impl.hpp>
#include <openstudio/model/GasEquipmentDefinition.hpp>
#include <openstudio/model/GasEquipmentDefinition_Impl.hpp>
#include <openstudio/model/SteamEquipmentDefinition.hpp>
#include <openstudio/model/SteamEquipmentDefinition_Impl.hpp>
#include <openstudio/model/OtherEquipmentDefinition.hpp>
#include <openstudio/model/OtherEquipmentDefinition_Impl.hpp>
#include <openstudio/model/WaterUseEquipmentDefinition.hpp>
#include <openstudio/model/WaterUseEquipmentDefinition_Impl.hpp>
#include <openstudio/model/PeopleDefinition.hpp>
#include <openstudio/model/PeopleDefinition_Impl.hpp>

#include <openstudio/utilities/core/Assert.hpp>

#include <QMessageBox>

#include <openstudio/utilities/core/Logger.hpp>

#include <openstudio/utilities/idd/IddEnums.hxx>

namespace openstudio {

LoadsController::LoadsController(bool isIP, const model::Model& model) : ModelSubTabController(new LoadsView(isIP, model), model) {
  connect(this, &LoadsController::toggleUnitsClicked, static_cast<ModelSubTabView*>(subTabView()), &ModelSubTabView::toggleUnitsClicked);
}

LoadsController::~LoadsController() = default;

void LoadsController::onAddObject(const openstudio::IddObjectType& iddObjectType) {
  switch (iddObjectType.value()) {
    case IddObjectType::OS_People_Definition:
      openstudio::model::PeopleDefinition(this->model());
      break;
    case IddObjectType::OS_InternalMass_Definition:
      openstudio::model::InternalMassDefinition(this->model());
      break;
    case IddObjectType::OS_Lights_Definition:
      openstudio::model::LightsDefinition(this->model());
      break;
    case IddObjectType::OS_Luminaire_Definition:
      openstudio::model::LuminaireDefinition(this->model());
      break;
    case IddObjectType::OS_ElectricEquipment_Definition:
      openstudio::model::ElectricEquipmentDefinition(this->model());
      break;
    case IddObjectType::OS_GasEquipment_Definition:
      openstudio::model::GasEquipmentDefinition(this->model());
      break;
    case IddObjectType::OS_SteamEquipment_Definition:
      openstudio::model::SteamEquipmentDefinition(this->model());
      break;
    case IddObjectType::OS_OtherEquipment_Definition:
      openstudio::model::OtherEquipmentDefinition(this->model());
      break;
    case IddObjectType::OS_WaterUse_Equipment_Definition:
      openstudio::model::WaterUseEquipmentDefinition(this->model());
      break;
    default:
      LOG_FREE(Error, "LoadsController", "Unknown IddObjectType '" << iddObjectType.valueName() << "'");
  }
}

void LoadsController::onCopyObject(const openstudio::model::ModelObject& modelObject) {
  modelObject.clone(this->model());
}

void LoadsController::onRemoveObject(openstudio::model::ModelObject modelObject) {
  boost::optional<model::SpaceLoadDefinition> spaceLoadDefinition = modelObject.optionalCast<model::SpaceLoadDefinition>();
  if (spaceLoadDefinition) {

    unsigned numInstances = spaceLoadDefinition->instances().size();
    if (numInstances > 0) {
      QMessageBox msgBox(subTabView());
      msgBox.setText("There are " + QString::number(numInstances) + " instances that reference this definition.");
      msgBox.setInformativeText("Do you want to remove this definition and all of its instances?");
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::Yes);
      int ret = msgBox.exec();
      if (ret == QMessageBox::No) {
        return;
      }
    }
    modelObject.remove();
  }
}

void LoadsController::onReplaceObject(openstudio::model::ModelObject modelObject, const OSItemId& replacementItemId) {
  // not yet implemented
}

void LoadsController::onPurgeObjects(const openstudio::IddObjectType& iddObjectType) {
  this->model().purgeUnusedResourceObjects(iddObjectType);
}

void LoadsController::onDrop(const OSItemId& itemId) {
  boost::optional<model::ModelObject> modelObject = this->getModelObject(itemId);
  if (modelObject) {
    if (modelObject->optionalCast<model::SpaceLoadDefinition>()) {
      if (this->fromComponentLibrary(itemId)) {
        modelObject->clone(this->model());
      }
    }
  }
}

void LoadsController::onInspectItem(OSItem* item) {}

void LoadsController::toggleUnits(bool displayIP) {}

}  // namespace openstudio
