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

#ifndef SHAREDGUICOMPONENTS_OSDIALOG_HPP
#define SHAREDGUICOMPONENTS_OSDIALOG_HPP

#include <QDialog>

class QMargins;

class QPoint;

class QPushButton;

class QSize;

class QVBoxLayout;

namespace openstudio {

class OSDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit OSDialog(bool isIP = false, QWidget* parent = nullptr);

  virtual ~OSDialog() = default;

  QSize sizeHint() const override;

  void setSizeHint(const QSize& sizeHint);

  QMargins layoutContentsMargins() const;

  void setLayoutContentsMargins(const QMargins& layoutContentsMargins);

  QVBoxLayout* upperLayout();

  QPushButton* backButton();

  QPushButton* cancelButton();

  QPushButton* okButton();

 protected:
  void setOkButtonAsDefault(bool isDefault);

  void mouseMoveEvent(QMouseEvent* event) override;

  void mouseReleaseEvent(QMouseEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;

  void resizeEvent(QResizeEvent* event) override;

  void paintEvent(QPaintEvent* event) override;

  bool m_isIP;

  QPushButton* m_backButton;

  QPushButton* m_cancelButton;

  QPushButton* m_okButton;

 private:
  virtual void createLayout();  // TODO: Chances are this doesn't need to be virtual...
  void createLayoutInternal();

  QVBoxLayout* m_upperLayout;

  QSize m_sizeHint;

  QMargins m_layoutContentsMargins;

  QPoint dragPosition;

  bool _move;

 signals:

  void backButtonClicked(bool checked);

  void cancelButtonClicked(bool checked);

  void okButtonClicked(bool checked);

  void toggleUnitsClicked(bool displayIP);

 protected slots:

  virtual void on_backButton(bool checked);

  virtual void on_cancelButton(bool checked);

  virtual void on_okButton(bool checked);

  virtual void toggleUnits(bool displayIP);
};

}  // namespace openstudio

#endif  // SHAREDGUICOMPONENTS_OSDIALOG_HPP
