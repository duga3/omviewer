#include <algorithm>
#include <iostream>

#include "ImageList.hpp"

using namespace cimg_library;

namespace omviewer {
ImageList::ImageList( const std::string& dir_name ):cur_i_( 0 ), dir_name_( dir_name )
{
	readFiles( dir_name_ );
}

CImg<float>ImageList::nextImage()
{
	cur_i_++;

	if( cur_i_ >= file_names_.size() )
	{
		cur_i_ = 0;
	}
	return getImage( cur_i_ );
}

CImg<float>ImageList::previousImage()
{
	cur_i_--;

	if( cur_i_ < 0 )
	{
		cur_i_ = file_names_.size() - 1;
	}
	return getImage( cur_i_ );
}

CImg<float>ImageList::getImage( size_t i ) const
{
	CImg<float> im( ( dir_name_ + file_names_.at( i ) ).c_str() );
	return im;
}

std::string ImageList::getCurrentFileName() const
{
	return file_names_.at( cur_i_ );
}

/*
 *	Private helper functions
 */
void ImageList::readFiles( const std::string& dir_name )
{
	// List all files in directory (default is ./)
	namespace fs =  boost::filesystem;
	fs::path image_directory( dir_name );
	fs::directory_iterator end_iter;
	std::string starting_file( "" );

	// actual file rather than dir
	if( !fs::is_directory( image_directory ) )
	{
		starting_file = image_directory.leaf().string();
		image_directory = image_directory.parent_path();
	}

	if( fs::exists( image_directory ) )
	{
		for( fs::directory_iterator dir_iter( image_directory ); dir_iter != end_iter; ++dir_iter )
		{
			if( fs::is_regular_file( dir_iter->status() ) )
			{
				for( auto& ext : supported_image_types_ )
				{
					if( dir_iter->path().extension() == ext )
					{
						file_names_.push_back( dir_iter->path().filename().string() );
					}
				}
			}
		}
	}

	dir_name_ = image_directory.string();

	if( dir_name_.back() != '/' )
	{
		dir_name_.push_back( '/' );
	}

	// Sort files since os doesn't guarantee ls-order
	std::sort( file_names_.begin(), file_names_.end() );

	// find index of starting file
	if( starting_file != "" )
	{
		cur_i_ = std::find( file_names_.begin(), file_names_.end(), starting_file ) - file_names_.begin();
	}
} // ImageList::readFiles
} // namespace
