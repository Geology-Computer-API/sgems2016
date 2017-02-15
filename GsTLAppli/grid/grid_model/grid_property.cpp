/**********************************************************************
** Author: Nicolas Remy
** Copyright (C) 2002-2004 The Board of Trustees of the Leland Stanford Junior
**   University
** All rights reserved.
**
** This file is part of the "grid" module of the Geostatistical Earth
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

#include <GsTLAppli/grid/grid_model/grid_property.h>
#include <GsTLAppli/grid/grid_model/grid_property_set.h>
#include <GsTLAppli/utils/string_manipulation.h>

#include <algorithm>
#include <stdio.h>
#include <QDomElement>

const float GsTLGridProperty::no_data_value = -9966699;



GsTLGridProperty::GsTLGridProperty( GsTLInt size, const std::string& name,
				    property_type default_value )
  : name_( name ), region_(NULL) {
  accessor_ = new MemoryAccessor( size, default_value );
}

GsTLGridProperty::GsTLGridProperty( GsTLInt size, const std::string& name,
			const std::string& in_filename, property_type default_value)
: name_( name ), region_(NULL) {
	accessor_ = new MemoryAccessor( size, default_value );
	std::ifstream file (in_filename.c_str(), std::ios::in|std::ios::binary);
	file.read ((char*)accessor_->data(), size*sizeof(float));
	//accessor_ = new DiskAccessor( size, name, in_filename );
}

GsTLGridProperty::~GsTLGridProperty() {
	std::vector<GsTLGridPropertyGroup*> groups = this->groups();
	for(int i=0; i<groups.size(); ++i ) {
		this->remove_group_membership(groups[i]->name());
	}
  delete accessor_;
}

void GsTLGridProperty::swap_to_disk() const {
  // If the property is already on disk, don't do anything.
  // Otherwise, create a DiskAccessor and delete the old Accessor.

  if( dynamic_cast<DiskAccessor*>( accessor_ ) ) return;
  
  DiskAccessor* new_accessor = new DiskAccessor( accessor_->size(), name_,
						 accessor_->data(),
						 accessor_->flags(0) );
  
  delete accessor_;
  accessor_ = new_accessor;
}


void GsTLGridProperty::swap_to_memory() const {
  DiskAccessor* current = dynamic_cast<DiskAccessor*>( accessor_ );
  if( !current ) return;
    
  MemoryAccessor* new_accessor = new MemoryAccessor( accessor_->size(),
						     current->stream() );
  delete accessor_;
  accessor_ = new_accessor;
}

bool GsTLGridProperty::is_in_memory() const{
	return dynamic_cast<MemoryAccessor*>( accessor_ );
}


//===========================================================
MemoryAccessor::MemoryAccessor( GsTLInt size ) {
#ifdef SGEMS_ACCESSOR_LARGE_FILE
// the data are store in arrrays of size of 1e6 values
  values_ =  new_data_structure( size);
#else
  values_ = new(std::nothrow) float[size];  
#endif
  flags_ = new(std::nothrow) bool[size];
  size_ = size;
#ifdef SGEMS_ACCESSOR_LARGE_FILE
  if(values_.empty()|| flags_ == NULL) size_ = 0;;
#else
   if(values_ == NULL || flags_ == NULL) size_ = 0;
#endif
 
}

MemoryAccessor::MemoryAccessor( GsTLInt size, float default_value ) {


#ifdef SGEMS_ACCESSOR_LARGE_FILE
// the data are store in arrrays of size of 1e6 values
  values_ =  new_data_structure( size);
#else
  values_ = new(std::nothrow) float[size];  
#endif

  flags_ = new(std::nothrow) bool[size];
  #ifdef SGEMS_ACCESSOR_LARGE_FILE
  if(values_.empty() || flags_ == NULL) size_ = 0;
  #else
  if(values_ == NULL || flags_ == NULL) size_ = 0;
  #endif
  else {
    size_ = size;
    for( GsTLInt i = 0; i < size ; i++ ) {
      #ifdef SGEMS_ACCESSOR_LARGE_FILE
        int arr_id = i/MEM_SIZE_ARRAY;
        int val_id = i - arr_id*MEM_SIZE_ARRAY;

        values_[arr_id][val_id] =  default_value;
      #else
        values_[i] = default_value; 
      #endif
     // values_[i] = default_value;
      flags_[i] = false;
    }
  }
}

MemoryAccessor::MemoryAccessor( GsTLInt size, std::fstream& stream ) {

#ifdef SGEMS_ACCESSOR_LARGE_FILE
// the data are store in arrrays of size of 1e6 values
  values_ =  new_data_structure( size);
#else
  values_ = new(std::nothrow) float[size];  
#endif

  flags_ = new(std::nothrow) bool[size];
#ifdef SGEMS_ACCESSOR_LARGE_FILE
 if(values_.empty() || flags_ == NULL) size_ = 0;
#else
   if(values_ == NULL || flags_ == NULL) size_ = 0;
#endif
 
  else {
    size_ = size;

    if( !stream.is_open() ) {
      GsTLlog << "Error: Stream not open when trying to swap property to memory!!"
              << gstlIO::end;
    }
    stream.seekg(0);

    long int remaining = size*sizeof( float );
    #ifdef SGEMS_ACCESSOR_LARGE_FILE
    for(int i=0; (i<values_.size()-1) ; i++ ) {
      stream.read( (char*) values_[i], MemoryAccessor::MEM_SIZE_ARRAY*sizeof( float ) );
    }
    stream.read( (char*) values_[values_.size()-1], last_array_length_*sizeof( float ) );
    #else
  //    long int remaining = size*sizeof( float );
      stream.read( (char*) values_, remaining );
    #endif


    
    // read the flags
    remaining = size*sizeof( bool );
    stream.read( (char*) flags_, remaining );
  }
}


MemoryAccessor::~MemoryAccessor() {
  #ifdef SGEMS_ACCESSOR_LARGE_FILE
  for(int i=0; i<values_.size(); i++)
    delete [] values_[i];
  #else
  delete [] values_;
  #endif
  
  delete [] flags_;
}


float MemoryAccessor::get_property_value( GsTLInt id ) {

  appli_assert( id >= 0 && id < size_ );
  #ifdef SGEMS_ACCESSOR_LARGE_FILE
    int arr_id = id/MEM_SIZE_ARRAY;
    int val_id = id - arr_id*MEM_SIZE_ARRAY;

    return values_[arr_id][val_id];
  #else
    return values_[id];

  #endif
  //return values_[id];
}

void MemoryAccessor::set_property_value( float val, GsTLInt id ) {
  appli_assert( id >= 0 && id < size_ );
  #ifdef SGEMS_ACCESSOR_LARGE_FILE
    int arr_id = id/MEM_SIZE_ARRAY;
    int val_id = id - arr_id*MEM_SIZE_ARRAY;

    values_[arr_id][val_id] = val;
  #else
    values_[id] = val;

  #endif
//  values_[id] = val;
}

// The current implementation only supports one set of flags
bool MemoryAccessor::get_flag( int, GsTLInt id ) {
  appli_assert( id >= 0 && id < size_ );
  return flags_[id];
}

// The current implementation only supports one set of flags
void MemoryAccessor::set_flag( bool flag, int, GsTLInt id ) {
  appli_assert( id >= 0 && id < size_ );
  flags_[id] = flag;
}

#ifdef SGEMS_ACCESSOR_LARGE_FILE
std::vector<float*> MemoryAccessor::data() {
    return values_;
  }

  const std::vector< float*> MemoryAccessor::data() const {
    return values_;
  }


/*
  float** MemoryAccessor::data() {
    return values_;
  }

  const float* const* MemoryAccessor::data() const {
    return values_;
  }
  */
#else
float* MemoryAccessor::data() {
  return values_;
}

const float* MemoryAccessor::data() const {
  return values_;
}
#endif




bool* MemoryAccessor::flags( int ) {
  return flags_;
}

const bool* MemoryAccessor::flags( int ) const {
  return flags_;
}
#ifdef SGEMS_ACCESSOR_LARGE_FILE
std::vector<float*> MemoryAccessor::new_data_structure(int size){
  int n_array = size/MEM_SIZE_ARRAY +1;
  values_.insert(values_.begin(),n_array,0);
//  for(int i=0; i < n_array; i++  ) values_[i] = 0;
  bool ok = true;
  for(int i=0; i < (n_array-1) ; i++  ) {
    values_[i] = new(std::nothrow) float[MEM_SIZE_ARRAY];
    if(values_[i] == 0) {
      ok = false;
      break;
    }
  }
  //Allocate the remainder
  if(ok) {
    last_array_length_ = size - (n_array-1)*MEM_SIZE_ARRAY ;
    values_[n_array-1] = new(std::nothrow) float[last_array_length_];
    if(values_[n_array-1] == 0) ok = false;

  }
  // free the allocate memory if needed
  if(!ok) {
     for(int i=0; i < n_array; i++  ) delete[] values_[i];
    // values_ = 0;
     values_.clear();
  }
  return values_;
}
#endif
//===============================================
DiskAccessor::DiskAccessor( GsTLInt size, const std::string& filename, 
			    const float* prop, const bool* flags ) 
  : buffer_size_( std::min( size, 10000 ) ) {

  cache_filename_ = DiskAccessor::cache_filename( filename );

  // if the file already exists, erase its content by opening it in write mode
  // (I don't know any other easy way to do that...)
  std::ofstream eraser( cache_filename_.c_str() );
  eraser.close();

  // Open a stream to the cache file in read/write mode
  cache_stream_.open( cache_filename_.c_str(), 
		      std::ios::in | std::ios::out | std::ios::binary );
  if( !cache_stream_ ) {
    GsTLcerr << "Can't write temporary file. Check that the directory is writable\n" 
             << "and that there is enough disk space left" << gstlIO::end;
  }


  flags_position_begin_ = static_cast<long int>( sizeof( float ) ) *
                          static_cast<long int>( size );

  val_bound_indexes_.first = -9;
  val_bound_indexes_.second = -9;
  flags_bound_indexes_.first = -9;
  flags_bound_indexes_.second = -9;
  val_buffer_modified_ = false;
  size_ = size;

  val_buffer_ = new float[buffer_size_];
  flags_buffer_ = new bool[buffer_size_];


  // If property values or flags were supplied, write them to file
  if( prop ) {
    long int remaining = static_cast<long int>( size_ ) * 
                         static_cast<long int>( sizeof(float) );
    cache_stream_.write( (char*) prop, remaining );
  }
  else {
    // write arbitrary values
    float arbitrary = -9966699;
    for(GsTLInt i=0; i< size; i++ )
      cache_stream_.write( (char*) &arbitrary, 1 );
  }

  if( flags ) {
    long int remaining = static_cast<long int>( size_ ) * 
                         static_cast<long int>( sizeof(bool) );
    cache_stream_.write( (char*) flags, remaining );
  }
  else {
    // write default flag value (false)
    bool default_value = false;
    for(GsTLInt i=0; i< size; i++ )
      cache_stream_.write( (char*) &default_value, 1 );
  }

  close_cache_stream();
}

//DiskAccessor::DiskAccessor( GsTLInt size, const std::string& filename, 
//			    const float* const* prop, const bool* flags )
#ifdef SGEMS_ACCESSOR_LARGE_FILE
DiskAccessor::DiskAccessor( GsTLInt size, const std::string& filename, 
                            const std::vector<float*> prop, const bool* flags ) 
  : buffer_size_( std::min( size, 10000 ) ) {

  cache_filename_ = DiskAccessor::cache_filename( filename );

  // if the file already exists, erase its content by opening it in write mode
  // (I don't know any other easy way to do that...)
  std::ofstream eraser( cache_filename_.c_str() );
  eraser.close();

  // Open a stream to the cache file in read/write mode
  cache_stream_.open( cache_filename_.c_str(), 
		      std::ios::in | std::ios::out | std::ios::binary );
  if( !cache_stream_ ) {
    GsTLcerr << "Can't write temporary file. Check that the directory is writable\n" 
             << "and that there is enough disk space left" << gstlIO::end;
  }


  flags_position_begin_ = static_cast<long int>( sizeof( float ) ) *
                          static_cast<long int>( size );

  val_bound_indexes_.first = -9;
  val_bound_indexes_.second = -9;
  flags_bound_indexes_.first = -9;
  flags_bound_indexes_.second = -9;
  val_buffer_modified_ = false;
  size_ = size;

  val_buffer_ = new float[buffer_size_];
  flags_buffer_ = new bool[buffer_size_];


  // If property values or flags were supplied, write them to file
  if( !prop.empty() ) {
    int n_array = size/MemoryAccessor::MEM_SIZE_ARRAY;
    for(int i=0; i<(n_array-1); i++) {
      long int remaining = static_cast<long int>( MemoryAccessor::MEM_SIZE_ARRAY ) * 
                           static_cast<long int>( sizeof(float) );
      cache_stream_.write( (char*) prop[i], remaining );
    }
    
    long int remaining = static_cast<long int>( size_ - n_array*MemoryAccessor::MEM_SIZE_ARRAY ) * 
                         static_cast<long int>( sizeof(float) );
    cache_stream_.write( (char*) prop[n_array], remaining );
  }
  else {
    // write arbitrary values
    float arbitrary = -9966699;
    for(GsTLInt i=0; i< size; i++ )
      cache_stream_.write( (char*) &arbitrary, 1 );
  }

  if( flags ) {
    long int remaining = static_cast<long int>( size_ ) * 
                         static_cast<long int>( sizeof(bool) );
    cache_stream_.write( (char*) flags, remaining );
  }
  else {
    // write default flag value (false)
    bool default_value = false;
    for(GsTLInt i=0; i< size; i++ )
      cache_stream_.write( (char*) &default_value, 1 );
  }

  close_cache_stream();
}
#endif
DiskAccessor::DiskAccessor( GsTLInt size, const std::string& filename,
						 const std::string& in_filename, const bool* flags )
	 : buffer_size_( std::min( size, 10000 ) ) {

	  cache_filename_ = DiskAccessor::cache_filename( filename );

	  // if the file already exists, erase its content by opening it in write mode
	  // (I don't know any other easy way to do that...)
	  std::ofstream eraser( cache_filename_.c_str() );
	  eraser.close();

	  // Open a stream to the cache file in read/write mode
	  cache_stream_.open( cache_filename_.c_str(),
			      std::ios::in | std::ios::out | std::ios::binary );
	  if( !cache_stream_ ) {
	    GsTLcerr << "Can't write temporary file. Check that the directory is writable\n"
	             << "and that there is enough disk space left" << gstlIO::end;
	  }


	  flags_position_begin_ = static_cast<long int>( sizeof( float ) ) *
	                          static_cast<long int>( size );

	  val_bound_indexes_.first = -9;
	  val_bound_indexes_.second = -9;
	  flags_bound_indexes_.first = -9;
	  flags_bound_indexes_.second = -9;
	  val_buffer_modified_ = false;
	  size_ = size;

	  val_buffer_ = new float[buffer_size_];
	  flags_buffer_ = new bool[buffer_size_];


	  std::fstream in_stream(in_filename.c_str());

	  // If property values or flags were supplied, write them to file
	  if( !in_stream.bad() ) {
	  	cache_stream_ << in_stream.rdbuf();
//	    long int remaining = static_cast<long int>( size_ ) *
//	                         static_cast<long int>( sizeof(float) );
//	    cache_stream_.write( (char*) in_stream., remaining );
	  }
	  else {
	    // write arbitrary values
	    float arbitrary = -9966699;
	    for(GsTLInt i=0; i< size; i++ )
	      cache_stream_.write( (char*) &arbitrary, 1 );
	  }

	  if( flags ) {
	    long int remaining = static_cast<long int>( size_ ) *
	                         static_cast<long int>( sizeof(bool) );
	    cache_stream_.write( (char*) flags, remaining );
	  }
	  else {
	    // write default flag value (false)
	    bool default_value = false;
	    for(GsTLInt i=0; i< size; i++ )
	      cache_stream_.write( (char*) &default_value, 1 );
	  }

	  close_cache_stream();

}


/*
DiskAccessor::DiskAccessor( GsTLInt size, const std::string& filename ) 
  : buffer_size_( std::min( size, 10000 ) ) {
  
  cache_filename_ = DiskAccessor::cache_filename( filename );

  flags_position_begin_ = static_cast<long int>( sizeof( float ) ) *
                          static_cast<long int>( size );

  val_bound_indexes_.first = -1;
  val_bound_indexes_.second = -1;
  flags_bound_indexes_.first = -1;
  flags_bound_indexes_.second = -1;

  size_ = size;

  val_buffer_ = new float[buffer_size_];
  flags_buffer_ = new bool[buffer_size_];
}
*/
DiskAccessor::~DiskAccessor() {
  //DiskAccessor::flush();  //useless to flush since we're deleting the file
  close_cache_stream();
  DiskAccessor::delete_cache_file();
  delete [] val_buffer_;
  delete [] flags_buffer_;
}

std::fstream& DiskAccessor::stream() {
  bool leave_open = true;
  DiskAccessor::flush( leave_open );
  open_cache_stream();

  return cache_stream_;
}

float DiskAccessor::get_property_value( GsTLInt id ) {
  appli_assert( id >= 0 && id < size_ );
  if( id >= val_bound_indexes_.first && id < val_bound_indexes_.second ) {
    return val_buffer_[id - val_bound_indexes_.first];
  }
  else {
    // flush the buffer, read the requested part of the file,
    // store it in the value buffer and return the property value
    bufferize_values( id );
    return val_buffer_[0];
  }
}


bool DiskAccessor::get_flag( int , GsTLInt id ) {
  appli_assert( id >= 0 && id < size_ );
  if( id >= flags_bound_indexes_.first && id < flags_bound_indexes_.second ) {
    return flags_buffer_[id - flags_bound_indexes_.first];
  }
  else {
    // flush the buffer, read the requested part of the file,
    // store it in the flags buffer and return the flag value.
    bufferize_flags( id );
    return flags_buffer_[0];
  }
}


void DiskAccessor::bufferize_values( GsTLInt start ) {
  bool leave_open = true;
  DiskAccessor::flush( leave_open );
  open_cache_stream();
  
  long int sizeof_float = static_cast<long int>( sizeof( float ) );
  long int pos = static_cast<long int>( start ) * sizeof_float;
  long int size = std::min( (long int) buffer_size_ * sizeof_float,
			    flags_position_begin_ - pos );		
  cache_stream_.seekg( pos );
  cache_stream_.read( (char*) val_buffer_, size );
  
  val_bound_indexes_.first = start;
  val_bound_indexes_.second = start + (cache_stream_.gcount() / sizeof(float) );

  close_cache_stream();
}

void DiskAccessor::bufferize_flags( GsTLInt start ) {
  bool leave_open = true;
  DiskAccessor::flush( leave_open );
  open_cache_stream();

  long int sizeof_bool = static_cast<long int>( sizeof( bool ) );
  long int pos = flags_position_begin_ + 
    static_cast<long int>( start ) * sizeof_bool;
  long int size = (long int) buffer_size_ * sizeof_bool;
			
  cache_stream_.seekg( pos );
  cache_stream_.read( (char*) flags_buffer_, size );
  
  flags_bound_indexes_.first = start;
  flags_bound_indexes_.second = start + (cache_stream_.gcount() / sizeof(bool) );

  close_cache_stream();
}




void DiskAccessor::flush( bool leave_stream_open ) {
  // if the buffer was modified write it to the file, otherwise do nothing
  if( val_buffer_modified_ ) {
    open_cache_stream();

    val_buffer_modified_ = false;
    long int pos = val_bound_indexes_.first * sizeof(float);
    int size = (val_bound_indexes_.second - val_bound_indexes_.first) * 
               sizeof(float);
    cache_stream_.seekp( pos );
    cache_stream_.write( (char*) val_buffer_, size );
  }

  if( flags_buffer_modified_ ) {
    open_cache_stream();

    flags_buffer_modified_ = false;
    long int pos = flags_bound_indexes_.first * sizeof(bool);
    int size = (flags_bound_indexes_.second - flags_bound_indexes_.first) * 
               sizeof(float);
    cache_stream_.seekp( flags_position_begin_ + pos );
    cache_stream_.write( (char*) flags_buffer_, size );
  }

  if( !leave_stream_open ) close_cache_stream();
}

int DiskAccessor::delete_cache_file() {
  int status = remove( cache_filename_.c_str() );
  return status;
}


std::string DiskAccessor::cache_filename( const std::string& prop_name ) {
  std::string basename( ".__sgems_" + prop_name + "__" );

  int id = 0;
  std::string name = basename;
  std::ifstream in( name.c_str() );
  while( in ) {
    in.close();
    name = basename + String_Op::to_string( id );
    in.open( name.c_str() );
    id++;
  }
  in.close();
  return name;
}


void DiskAccessor::set_property_value( float val, GsTLInt id) {
  appli_assert( id >= 0 && id < size_ );
  if( id >= val_bound_indexes_.first && id < val_bound_indexes_.second ) {
    val_buffer_[id - val_bound_indexes_.first] = val;
    val_buffer_modified_ = true;
  }
  else {
    // flush the buffer, read the requested part of the file,
    // store it in the flags buffer and return the flag value.
    bufferize_values( id );
    val_buffer_[0] = val;
  }
}



void DiskAccessor::set_flag( bool flag, int flag_id, GsTLInt id ) {
  appli_assert( id >= 0 && id < size_ );
  if( id >= flags_bound_indexes_.first && id < flags_bound_indexes_.second ) {
    flags_buffer_[id - flags_bound_indexes_.first] = flag;
    flags_buffer_modified_ = true;
  }
  else {
    // flush the buffer, read the requested part of the file,
    // store it in the flags buffer and return the flag value.
    bufferize_flags( id );
    flags_buffer_[0] = flag;
  }
  
}


void DiskAccessor::open_cache_stream() {
  if( cache_stream_.is_open() ) return;

  cache_stream_.open( cache_filename_.c_str(),
            		      std::ios::in | std::ios::out | std::ios::binary );
}

void DiskAccessor::close_cache_stream() {
  if( cache_stream_.is_open() ) cache_stream_.close();
}


//=====================================

GsTLGridProperty::iterator::iterator( GsTLGridProperty* prop, GsTLInt id, bool skip )
  : prop_(prop), id_(id), skip_uninformed_( skip ) {
  max_id_ = prop_->size();
 
  // if the current id is not informed, advance to the next informed id
  // if it is outside the active region. move to the next node
  if( id_ < max_id_ && skip ) {
    while( !(prop_->is_inside_region(id_) && prop_->is_informed(id_)) && id_ < prop_->size() ) { id_++; }
  }
}


GsTLGridProperty::const_iterator::const_iterator( GsTLGridProperty::iterator it )
  : prop_(it.prop_), id_(it.id_), skip_uninformed_( skip_uninformed_ ) {
  max_id_ = prop_->size();
 
  // if the current id is not informed, advance to the next informed id 
  if( id_ < max_id_ && skip_uninformed_ ) {
    while( !(prop_->is_inside_region(id_) && prop_->is_informed(id_) ) && id_ < prop_->size() ) { id_++; }
  }
}



GsTLGridProperty::const_iterator::
const_iterator( const GsTLGridProperty* prop, GsTLInt id, bool skip )
  : prop_(prop), id_(id), skip_uninformed_( skip ) {
  max_id_ = prop_->size();
 
  // if the current id is not informed, advance to the next informed id 
  if( id_ < max_id_ && skip ) {
    while( !(prop_->is_inside_region(id_) && prop_->is_informed(id_)) && id_ < prop_->size() ) { id_++; }
  }
}



bool GsTLGridProperty::has_group_membership() const{
  return !groups_.empty();
}
inline 
std::vector< std::pair<std::string,std::string> > GsTLGridProperty::group_pair_name_type() const{
  std::vector< std::pair<std::string,std::string> > group_ids;
  std::vector<  GsTLGridPropertyGroup* >::const_iterator it = groups_.begin();
  for(; it != groups_.end(); ++it) {
    std::pair<std::string, std::string > id( (*it)->name(), (*it)->type() );
    group_ids.push_back( id );
  }
  return group_ids;
}

std::vector<std::string > GsTLGridProperty::group_names() const{
  std::vector<std::string> names;
  std::vector<  GsTLGridPropertyGroup* >::const_iterator it = groups_.begin();
  for(; it != groups_.end(); ++it) {
    names.push_back( (*it)->name() );
  }
  return names;
}


std::vector<std::string > GsTLGridProperty::group_types() const{
  std::vector<std::string> types;
  std::vector< GsTLGridPropertyGroup* >::const_iterator it = groups_.begin();
  for(; it != groups_.end(); ++it) {
    types.push_back( (*it)->type() );
  }
  return types;
}


 std::vector< GsTLGridPropertyGroup*>& GsTLGridProperty::groups() {
  return groups_;
}


bool GsTLGridProperty::add_group_membership(GsTLGridPropertyGroup* group) {
  std::vector<  GsTLGridPropertyGroup* >::iterator it = groups_.begin();
  std::string gname = group->name();
  for(; it != groups_.end(); ++it) {
    if(gname == (*it)->name()) return false;
  }  
  groups_.push_back(group);
  return true;
}


bool GsTLGridProperty::remove_group_membership(GsTLGridPropertyGroup* group){
  return remove_group_membership(group->name());
}

bool GsTLGridProperty::remove_group_membership(const std::string& group_name){
  std::vector<  GsTLGridPropertyGroup* >::iterator it = groups_.begin();
  for(; it != groups_.end(); ++it) {
    if(group_name == (*it)->name()) {
      groups_.erase(it);
      return true;
    }
  }  
  return false;
}

int GsTLGridProperty::number_group_membership() const{
  return groups_.size();
}

/*
bool GsTLGridPropertyGroup::add_property(GsTLGridProperty* prop) {
  properties_.insert(prop);
  return true;
}

bool GsTLGridPropertyGroup::remove_property(GsTLGridProperty* prop){
    unsigned int ok = properties_.erase( prop );
    return ok != 0;
}

std::vector<GsTLGridProperty::property_type> 
GsTLGridPropertyGroup::get_vector_data( int node_id ){
  property_set::iterator it = properties_.begin();
  std::vector<GsTLGridProperty::property_type> values;
  values.reserve(properties_.size() );
  for( ; it!= properties_.end(); ++it) {
    if((*it)->is_informed(node_id) ) values.push_back( (*it)->get_value(node_id) );
  }
  return values;
}
*/
