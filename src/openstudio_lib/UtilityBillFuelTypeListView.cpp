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

#include "UtilityBillFuelTypeListView.hpp"

#include "ModelObjectItem.hpp"
#include "OSItemList.hpp"
#include "OSAppBase.hpp"

#include <openstudio/model/Model_Impl.hpp>
#include <openstudio/model/ModelObject_Impl.hpp>
#include <openstudio/model/UtilityBill.hpp>
#include <openstudio/model/UtilityBill_Impl.hpp>

#include "../model_editor/Utilities.hpp"

#include <openstudio/utilities/core/Assert.hpp>

#include <iostream>

namespace openstudio {

UtilityBillFuelTypeListController::UtilityBillFuelTypeListController(const model::Model& model, const openstudio::FuelType& fuelType)
  : m_iddObjectType(model::UtilityBill::iddObjectType()), m_fuelType(fuelType), m_model(model) {
  //model.getImpl<model::detail::Model_Impl>().get()->addWorkspaceObjectPtr.connect<UtilityBillFuelTypeListController, &UtilityBillFuelTypeListController::objectAdded>(this);
  connect(OSAppBase::instance(), &OSAppBase::workspaceObjectAddedPtr, this, &UtilityBillFuelTypeListController::objectAdded, Qt::QueuedConnection);

  //model.getImpl<model::detail::Model_Impl>().get()->removeWorkspaceObjectPtr.connect<UtilityBillFuelTypeListController, &UtilityBillFuelTypeListController::objectRemoved>(this);
  connect(OSAppBase::instance(), &OSAppBase::workspaceObjectRemovedPtr, this, &UtilityBillFuelTypeListController::objectRemoved,
          Qt::QueuedConnection);
}

IddObjectType UtilityBillFuelTypeListController::iddObjectType() const {
  return m_iddObjectType;
}

FuelType UtilityBillFuelTypeListController::fuelType() const {
  return m_fuelType;
}

void UtilityBillFuelTypeListController::objectAdded(std::shared_ptr<openstudio::detail::WorkspaceObject_Impl> impl,
                                                    const openstudio::IddObjectType& iddObjectType, const openstudio::UUID& handle) {
  if (iddObjectType == m_iddObjectType) {
    // in a ModelObjectTypeListView this is sufficient to say that a new item has been added to our list
    // however, in this case we need to also check the fuel type
    if (std::dynamic_pointer_cast<model::detail::UtilityBill_Impl>(impl)) {
      if (std::dynamic_pointer_cast<model::detail::UtilityBill_Impl>(impl)->fuelType() == m_fuelType) {

        std::vector<OSItemId> ids = this->makeVector();
        emit itemIds(ids);

        for (const OSItemId& id : ids) {
          if (id.itemId() == toQString(impl->handle())) {
            emit selectedItemId(id);
            break;
          }
        }
      }
    }
  }
}

void UtilityBillFuelTypeListController::objectRemoved(std::shared_ptr<openstudio::detail::WorkspaceObject_Impl> impl,
                                                      const openstudio::IddObjectType& iddObjectType, const openstudio::UUID& handle) {
  if (iddObjectType == m_iddObjectType) {
    // in a ModelObjectTypeListView this is sufficient to say that a new item has been added to our list
    // however, in this case we need to also check the fuel type
    if (std::dynamic_pointer_cast<model::detail::UtilityBill_Impl>(impl)) {
      if (std::dynamic_pointer_cast<model::detail::UtilityBill_Impl>(impl)->fuelType() == m_fuelType) {

        emit itemIds(makeVector());
      }
    }
  }
}

std::vector<OSItemId> UtilityBillFuelTypeListController::makeVector() {
  std::vector<OSItemId> result;

  // get objects by type
  std::vector<WorkspaceObject> workspaceObjects = m_model.getObjectsByType(m_iddObjectType);

  // sort by name
  std::sort(workspaceObjects.begin(), workspaceObjects.end(), WorkspaceObjectNameGreater());

  for (const WorkspaceObject& workspaceObject : workspaceObjects) {
    if (!workspaceObject.handle().isNull()) {
      auto modelObject = workspaceObject.cast<openstudio::model::ModelObject>();
      if (boost::optional<model::UtilityBill> utilityBill = modelObject.optionalCast<model::UtilityBill>()) {
        if (utilityBill.get().fuelType() == m_fuelType) {
          result.push_back(modelObjectToItemId(modelObject, false));
          // because there is no more than 1 utility bill per fuel type...
          // TODO break;
        }
      }
    }
  }

  return result;
}

UtilityBillFuelTypeListView::UtilityBillFuelTypeListView(const model::Model& model, const openstudio::FuelType& fuelType, bool addScrollArea,
                                                         QWidget* parent)
  : OSItemList(new UtilityBillFuelTypeListController(model, fuelType), addScrollArea), m_fuelType(fuelType) {}

boost::optional<openstudio::model::ModelObject> UtilityBillFuelTypeListView::selectedModelObject() const {
  OSItem* selectedItem = this->selectedItem();
  auto* modelObjectItem = qobject_cast<ModelObjectItem*>(selectedItem);
  if (modelObjectItem) {
    return modelObjectItem->modelObject();
  }
  return boost::none;
}

IddObjectType UtilityBillFuelTypeListView::iddObjectType() const {
  OSVectorController* vectorController = this->vectorController();
  auto* utilityBillListController = qobject_cast<UtilityBillFuelTypeListController*>(vectorController);
  OS_ASSERT(utilityBillListController);
  return utilityBillListController->iddObjectType();
}

FuelType UtilityBillFuelTypeListView::fuelType() const {
  return m_fuelType;
}

}  // namespace openstudio
