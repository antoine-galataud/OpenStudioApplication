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

#ifndef OPENSTUDIO_CONSTRUCTIONSVIEW_HPP
#define OPENSTUDIO_CONSTRUCTIONSVIEW_HPP

#include "ModelObjectInspectorView.hpp"
#include "ModelSubTabView.hpp"

#include <openstudio/model/Model.hpp>

namespace openstudio {

class ConstructionsView : public ModelSubTabView
{
  Q_OBJECT

 public:
  ConstructionsView(bool isIP, const openstudio::model::Model& model, QWidget* parent = nullptr);

  virtual ~ConstructionsView() {}

 private:
  static std::vector<std::pair<IddObjectType, std::string>> modelObjectTypesAndNames();
};

class ConstructionsInspectorView : public ModelObjectInspectorView
{
  Q_OBJECT

 public:
  ConstructionsInspectorView(bool isIP, const model::Model& model, QWidget* parent = nullptr);

  virtual ~ConstructionsInspectorView() {}

 protected:
  virtual void onClearSelection() override;

  virtual void onSelectModelObject(const openstudio::model::ModelObject& modelObject) override;

  virtual void onUpdate() override;

 private:
  void showConstructionInspector(const openstudio::model::ModelObject& modelObject);

  void showAirBoundaryInspector(const openstudio::model::ModelObject& modelObject);

  void showCfactorUndergroundWallInspector(const openstudio::model::ModelObject& modelObject);

  void showFfactorGroundFloorInspector(const openstudio::model::ModelObject& modelObject);

  void showInternalSourceInspector(const openstudio::model::ModelObject& modelObject);

  void showWindowDataFileInspector(const openstudio::model::ModelObject& modelObject);

  void showDefaultView();

  void showInspector(QWidget* widget);

  std::map<openstudio::IddObjectType, int> m_inspectorIndexMap;

  bool m_isIP;

 public slots:

  void toggleUnits(bool displayIP) override;
};

}  // namespace openstudio

#endif  // OPENSTUDIO_CONSTRUCTIONSVIEW_HPP
