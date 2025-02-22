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

#ifndef OPENSTUDIO_THERMALZONESVIEW_HPP
#define OPENSTUDIO_THERMALZONESVIEW_HPP

#include "ModelObjectInspectorView.hpp"
#include "ModelSubTabView.hpp"

#include <openstudio/model/ThermalZone.hpp>

namespace openstudio {

class ThermalZonesGridView;

class ThermalZonesView : public ModelSubTabView
{
  Q_OBJECT

 public:
  ThermalZonesView(bool isIP, const model::Model& model, QWidget* parent = nullptr);

  virtual ~ThermalZonesView() {}
};

class ThermalZoneView : public ModelObjectInspectorView
{

  Q_OBJECT

 public:
  ThermalZoneView(bool isIP, const model::Model& model, QWidget* parent = nullptr);

  virtual ~ThermalZoneView() {}

  virtual bool supportsMultipleObjectSelection() const override {
    return true;
  }
  virtual std::set<model::ModelObject> selectedObjects() const override;

 public slots:

  void refresh();

 signals:

  void modelObjectSelected(model::OptionalModelObject& modelObject, bool readOnly);

  void dropZoneItemSelected(OSItem* item, bool readOnly);

 protected:
  void onClearSelection() override;

  void onSelectModelObject(const openstudio::model::ModelObject& modelObject) override;

  void onUpdate() override;

 private slots:

  void toggleUnits(bool) override;

 private:
  ThermalZonesGridView* m_thermalZonesGridView = nullptr;

  bool m_isIP;
};

}  // namespace openstudio

#endif  // OPENSTUDIO_THERMALZONESVIEW_HPP
