/**********************************************************************
** Author: Nicolas Remy
** Copyright (C) 2002-2004 The Board of Trustees of the Leland Stanford Junior
**   University
** All rights reserved.
**
** This file is part of the "filters" module of the Geostatistical Earth
** Modeling Software (GEMS)
**
** This file may be distributed and/or modified under the terms of the 
** license defined by the Stanford Center for Reservoir Forecasting and 
** appearing in the file LICENSE.XFREE included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.gnu.org/copyleft/gpl.html for GPL licensing information.
**
** Contact the Stanford Center for Reservoir Forecasting, Stanford University
** if any conditions of this licensing are not clear to you.
**
**********************************************************************/

#include <GsTLAppli/filters/gslib/filter_qt_dialogs.h>
#include <GsTLAppli/filters/gslib/gslib_filter.h>
#include <GsTLAppli/utils/gstl_messages.h>
#include <GsTLAppli/appli/manager_repository.h>

#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qmessagebox.h>


#include <iostream>



Gslib_infilter_dialog::Gslib_infilter_dialog( QWidget* parent, const char* name )
: QDialog(parent),
  filter_( 0 ), 
  file_stream_( 0 ) {

  setupUi(this);
  SmartPtr<Named_interface> ni = 
      Root::instance()->interface( gslibInputFilters_manager );
  Manager* mng = dynamic_cast<Manager*>( ni.raw_ptr() );
  appli_assert( mng );

  Manager::type_iterator begin = mng->begin();
  Manager::type_iterator end = mng->end();
  for( ; begin != end ; ++begin ) {
    ObjectType->addItem( QString( begin->c_str() ) );
  }

  create_specialized_filter( ObjectType->currentText() );
  _back->setEnabled(false);
  
  ObjectType->setFocus();

  QObject::connect( ObjectType, SIGNAL(activated( const QString& )), 
		    this, SLOT( create_specialized_filter( const QString& ) ) );
  QObject::connect( _next, SIGNAL(clicked()), this, SLOT(nextClicked()));
  QObject::connect( _back, SIGNAL(clicked()), this, SLOT(backClicked()));
  QObject::connect( _cancel, SIGNAL(clicked()), this, SLOT(reject()));

}


Gslib_infilter_dialog::~Gslib_infilter_dialog(){
}

void Gslib_infilter_dialog::nextClicked()
{
  if (_next->text() == "Finish")
    accept();
  else {
    _back->setEnabled(true);
    _next->setText("Finish");
    stackedWidget->setCurrentIndex(1);
  }
}

void Gslib_infilter_dialog::backClicked()
{
  _next->setText("Next>");
  stackedWidget->setCurrentIndex(0);
  _back->setEnabled(false);
}

void Gslib_infilter_dialog::set_file( std::ifstream& infile ) {
  file_stream_ = &infile;

  std::streampos mark = infile.tellg();

  QString text;
  std::string line;

  int count = 0;
  const int limit = 50;
 
  // Read up to "limit" lines
  while( std::getline( infile, line ) && count < limit ) {
    line += "\n";
    text.append( line.c_str() );
    count ++;
  } 
    
  text_preview_->setPlainText( text );
  
  // set the stream back to where it was
  infile.clear();
  infile.seekg( mark );
}


void 
Gslib_infilter_dialog::create_specialized_filter( const QString& obj_type ) {
  if( obj_type.isEmpty() ) return;

  QByteArray tmp = obj_type.toLatin1();
  std::string type( tmp.constData() );

  if( filter_ ) {
    if( filter_->object_filtered() == type ) return;
  }


  _next->setFocus();

  SmartPtr<Named_interface> ni = 
      Root::instance()->interface( gslibInputFilters_manager + "/" + type );
  
  if( ni.raw_ptr() == 0 ) {
    ni = Root::instance()->new_interface( type,
					  gslibInputFilters_manager + "/" + type );
  }

  filter_ = dynamic_cast<Gslib_specialized_infilter*>( ni.raw_ptr() );
  appli_assert( filter_ );

  // If a filter previously added a new page to the wizard, remove that page
  if( stackedWidget->count() > 1 )
    stackedWidget->removeWidget( stackedWidget->widget(1) );

  // ask the specialized filter to give its page widget. If widget is 0, 
  // enable Finish (ie there is no need for additional information).
  // if != 0, enable Next. 

  QWidget* page1_ = filter_->init_dialog( *file_stream_ );
  stackedWidget->addWidget(page1_);

  
  /*
  if( page1_ == 0 ) {
    setFinishEnabled( currentPage(), true );
  }
  else {
    stackedWidget->addWidget(page1_);
    setFinishEnabled( page(1), true );
    finishButton()->setFocus();
  }
  */
  
}


void Gslib_infilter_dialog::accept() {
  if( !filter_->has_valid_parameters() ) {
    QMessageBox::warning( this, "Missing parameters", 
                          "A name for the new object must be provided",
                          QMessageBox::Ok, Qt::NoButton,
                          Qt::NoButton );
  }
  else {
    QDialog::accept();
  }

}



//==============================================================
Gslib_input_mgrid_dialog::Gslib_input_mgrid_dialog( QWidget* parent, 
												   const char* name ) 
 : QWidget( parent )
{
        setupUi(this);
	name_->setFocus();
}

float Gslib_input_mgrid_dialog::x_size() const {
  QString val = x_size_->text();
  return val.toFloat();
}
float Gslib_input_mgrid_dialog::y_size() const{
  QString val = y_size_->text();
  return val.toFloat();
}
float Gslib_input_mgrid_dialog::z_size() const{
  QString val = z_size_->text();
  return val.toFloat();
}
float Gslib_input_mgrid_dialog::Ox() const{
  QString val = xmn_->text();
  return val.toFloat();
}
float Gslib_input_mgrid_dialog::Oy() const{
  QString val = ymn_->text();
  return val.toFloat();
}
float Gslib_input_mgrid_dialog::Oz() const{
  QString val = zmn_->text();
  return val.toFloat();
}

float Gslib_input_mgrid_dialog::nx() const{
  return nx_->value();
}
float Gslib_input_mgrid_dialog::ny() const{
  return ny_->value();
}
float Gslib_input_mgrid_dialog::nz() const{
  return nz_->value();
}

QString Gslib_input_mgrid_dialog::name() const {
  return name_->text();
}

int Gslib_input_mgrid_dialog::X_column() const {
  return X_col_->value();
}
int Gslib_input_mgrid_dialog::Y_column() const {
  return Y_col_->value();
}
int Gslib_input_mgrid_dialog::Z_column() const {
  return Z_col_->value();
}

int Gslib_input_mgrid_dialog::mask_column() const {
  return mask_col_->value();
}



bool Gslib_input_mgrid_dialog::use_no_data_value() const {
  return use_no_data_value_->isChecked();
}

float Gslib_input_mgrid_dialog::no_data_value() const {
  QString val = no_data_value_edit_->text();
  if(val.isEmpty()) return -9966699;
  return val.toFloat();
}

//==============================================================

Gslib_input_grid_dialog::Gslib_input_grid_dialog( QWidget* parent, 
						  const char* name )
  : QWidget( parent ) {
  setupUi(this);
  name_->setFocus();
}

int Gslib_input_grid_dialog::nx() const {
  return nx_->value();
}

int Gslib_input_grid_dialog::ny() const {
  return ny_->value();
}

int Gslib_input_grid_dialog::nz() const {
  return nz_->value();
}


float Gslib_input_grid_dialog::x_size() const {
  QString val = x_size_->text();
  return val.toFloat();
}
float Gslib_input_grid_dialog::y_size() const{
  QString val = y_size_->text();
  return val.toFloat();
}
float Gslib_input_grid_dialog::z_size() const{
  QString val = z_size_->text();
  return val.toFloat();
}
float Gslib_input_grid_dialog::Ox() const{
  QString val = xmn_->text();
  return val.toFloat();
}
float Gslib_input_grid_dialog::Oy() const{
  QString val = ymn_->text();
  return val.toFloat();
}
float Gslib_input_grid_dialog::Oz() const{
  QString val = zmn_->text();
  return val.toFloat();
}

QString Gslib_input_grid_dialog::name() const {
  return name_->text();
}


bool Gslib_input_grid_dialog::use_no_data_value() const {
  return use_no_data_value_->isChecked();
}

float Gslib_input_grid_dialog::no_data_value() const {
  QString val = no_data_value_edit_->text();
  return val.toFloat();
}


//==========================================================

Gslib_input_pointset_dialog::Gslib_input_pointset_dialog( QWidget* parent , 
							  const char* name ) 
  : QWidget( parent ) {
  setupUi(this);
  name_->setFocus();
}
  
Gslib_input_pointset_dialog::Gslib_input_pointset_dialog( std::ifstream& infile,
							  QWidget* parent , 
							  const char* name ) 
  : QWidget( parent) {

  setupUi(this);
  std::streampos mark = infile.tellg();

  // read how many properties are defined in the file (indicated at the
  // second line of the file)
  std::string line;

  std::getline( infile, line );
  int nb_properties;
  infile >> nb_properties;
  infile.clear();
  infile.seekg( mark );


  X_col_->setMaximum( nb_properties );
  Y_col_->setMaximum( nb_properties );
  Z_col_->setMaximum( nb_properties );

  
  // make a preview of the file

  QString text;

  int count = 0;
  const int limit = 50;
 
  // Read up to "limit" lines
  while( std::getline( infile, line ) && count < limit ) {
    line += "\n";
    text.append( line.c_str() );
    count ++;
  } 
    
  text_preview_->setPlainText( text );
  
  // set the stream back to where it was
  infile.clear();
  infile.seekg( mark );

  name_->setFocus();
}
  

QString Gslib_input_pointset_dialog::name() const {
  return name_->text();
}
int Gslib_input_pointset_dialog::X_column() const {
  return X_col_->value();
}
int Gslib_input_pointset_dialog::Y_column() const {
  return Y_col_->value();
}
int Gslib_input_pointset_dialog::Z_column() const {
  return Z_col_->value();
}
  

bool Gslib_input_pointset_dialog::use_no_data_value() const {
  return use_no_data_value_->isChecked();
}

float Gslib_input_pointset_dialog::no_data_value() const {
  QString val = no_data_value_edit_->text();
  return val.toFloat();
}


