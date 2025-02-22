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

#ifndef SHAREDGUICOMPONENTS_OSCHECKBOX_HPP
#define SHAREDGUICOMPONENTS_OSCHECKBOX_HPP

#include "FieldMethodTypedefs.hpp"

#include <openstudio/model/Model.hpp>

#include <QCheckBox>
#include <openstudio/nano/nano_signal_slot.hpp>  // Signal-Slot replacement
#include <QPushButton>

// Forward declaration
class QFocusEvent;

namespace openstudio {

class OSSelectAllCheckBox : public QCheckBox
{
  Q_OBJECT

 public:
  explicit OSSelectAllCheckBox(QWidget* parent = nullptr);

  virtual ~OSSelectAllCheckBox();

 public slots:

  void onGridRowSelectionChanged(int numSelected, int numSelectable);
};

class OSCheckBox3
  : public QCheckBox
  , public Nano::Observer
{
  Q_OBJECT

 public:
  explicit OSCheckBox3(QWidget* parent = nullptr);

  virtual ~OSCheckBox3();

  // This method will be called to enable the Checkbox to accept focus
  // (typically by the OSGridController depending on whether the underlying BaseConcept allows it)
  void enableClickFocus();

  void disableClickFocus();

  bool locked() const;

  void setLocked(bool locked);

  void bind(const model::ModelObject& modelObject, BoolGetter get, boost::optional<BoolSetter> set = boost::none,
            boost::optional<NoFailAction> reset = boost::none, boost::optional<BasicQuery> isDefaulted = boost::none);

  void bind(const model::ModelObject& modelObject, BoolGetter get, boost::optional<BoolSetterBoolReturn> set = boost::none,
            boost::optional<NoFailAction> reset = boost::none, boost::optional<BasicQuery> isDefaulted = boost::none);

  void unbind();

 signals:

  void inFocus(bool inFocus, bool hasData);

 protected:
  // We override these methods to emit inFocus as appropriate to enable/disable the header button
  virtual void focusInEvent(QFocusEvent* e) override;
  virtual void focusOutEvent(QFocusEvent* e) override;

 private slots:

  void onToggled(bool checked);

  void onModelObjectChange();

  void onModelObjectRemove(const Handle& handle);

 private:
  bool defaulted() const;
  void updateStyle();

  boost::optional<model::ModelObject> m_modelObject;
  boost::optional<BoolGetter> m_get;
  boost::optional<BoolSetter> m_set;
  boost::optional<BoolSetterBoolReturn> m_setBoolReturn;
  boost::optional<NoFailAction> m_reset;
  boost::optional<BasicQuery> m_isDefaulted;

  bool m_hasClickFocus = false;
  bool m_focused = false;
  bool m_locked = false;
};

class OSCheckBox2
  : public QPushButton
  , public Nano::Observer
{
  Q_OBJECT

 public:
  explicit OSCheckBox2(QWidget* parent = nullptr);

  virtual ~OSCheckBox2() {}

  void bind(const model::ModelObject& modelObject, BoolGetter get, boost::optional<BoolSetter> set = boost::none,
            boost::optional<NoFailAction> reset = boost::none, boost::optional<BasicQuery> isDefaulted = boost::none);

  void unbind();

 signals:

  void inFocus(bool inFocus, bool hasData);

 protected:
  // We override these methods to emit inFocus as appropriate to enable/disable the header button
  virtual void focusInEvent(QFocusEvent* e) override;
  virtual void focusOutEvent(QFocusEvent* e) override;

 private slots:

  void onToggled(bool checked);

  void onModelObjectChange();

  void onModelObjectRemove(const Handle& handle);

 private:
  bool defaulted() const;
  void updateStyle();

  boost::optional<model::ModelObject> m_modelObject;
  boost::optional<BoolGetter> m_get;
  boost::optional<BoolSetter> m_set;
  boost::optional<NoFailAction> m_reset;
  boost::optional<BasicQuery> m_isDefaulted;

  bool m_hasClickFocus = false;
  bool m_focused = false;
  bool m_locked = false;
};

class OSGreyCheckBox2
  : public QPushButton
  , public Nano::Observer
{
  Q_OBJECT

 public:
  explicit OSGreyCheckBox2(QWidget* parent = nullptr);

  virtual ~OSGreyCheckBox2() {}

  void bind(const model::ModelObject& modelObject, BoolGetter get, boost::optional<BoolSetter> set = boost::none);

  void unbind();

 private slots:

  void onToggled(bool checked);

  void onModelObjectChange();

  void onModelObjectRemove(const Handle& handle);

 private:
  boost::optional<model::ModelObject> m_modelObject;
  boost::optional<BoolGetter> m_get;
  boost::optional<BoolSetter> m_set;
};

}  // namespace openstudio

#endif  // SHAREDGUICOMPONENTS_OSCHECKBOX_HPP
