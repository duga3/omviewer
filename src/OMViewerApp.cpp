#include "OMViewerApp.hpp"

#include <CImg.h>

using namespace cimg_library;

namespace omviewer {
void OMViewerApp::run()
{
	cur_im_ = im_list_.getImage( 0 );
	
	CImgDisplay main_disp( cur_im_ );
	main_disp.set_title( im_list_.getCurrentFileName().c_str() );
	equalize_depth_image_ = cur_im_.max() > 255 ? true : false;
	histogram_equalize_ = false;
	display( main_disp );
}

void OMViewerApp::resetView( cimg_library::CImgDisplay& disp )
{
	XYZ_[0]			= ( upper_left_[0] + lower_right_[0] ) / 2;
	XYZ_[1]			= ( upper_left_[1] + lower_right_[1] ) / 2;
	XYZ_[2]			= ( upper_left_[2] + lower_right_[2] ) / 2;
	upper_left_[0]	= upper_left_[1] = upper_left_[2] = 0;
	lower_right_[0] = cur_im_.width() - 1;
	lower_right_[1] = cur_im_.height() - 1;
	lower_right_[2] = cur_im_.depth() - 1;
	old_w_			= disp.width();
	old_h_			= disp.height();
} // OMViewerApp::resetView

void OMViewerApp::resizeView( cimg_library::CImgDisplay& disp, unsigned int tw, unsigned int th )
{
	const unsigned int ttw	= tw * disp.width() / old_w_;
	const unsigned int tth	= th * disp.height() / old_h_;
	const unsigned int dM	= cimg::max( ttw, tth ), diM = (unsigned int)cimg::max( disp.width(), disp.height() );
	const unsigned int imgw = cimg::max( 16U, ttw * diM / dM );
	const unsigned int imgh = cimg::max( 16U, tth * diM / dM );

	disp.set_fullscreen( false ).resize( cimg_fitscreen( imgw, imgh, 1 ), false );
}

int OMViewerApp::getSelection( const cimg_library::CImg<float>& subim, cimg_library::CImgDisplay& disp )
{
	const int  featuretype	 = 2;
	const bool reset_3d_view = false;
	bool has_depth_channels	 = subim.depth() > 1;

	// Magic cimg funciton for getting display
	const CImg<cimg::last<float, int>::type> selection = subim._get_select( disp,
																			nullptr,
																			featuretype,
																			XYZ_,
																			upper_left_[0],
																			upper_left_[1],
																			upper_left_[2],
																			reset_3d_view,
																			has_depth_channels );
	if( disp.wheel() )
	{
		if( disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT() )
		{
			if( disp.wheel() > 0 )
			{
				view_action_ = ViewAction::zoom_in;
			}
			else
			{
				view_action_ = ViewAction::zoom_out;
			}
		}
		else if( disp.is_keySHIFTLEFT() || disp.is_keySHIFTRIGHT() )
		{
			if( disp.wheel() > 0 )
			{
				view_action_ = ViewAction::go_left;
			}
			else
			{
				view_action_ = ViewAction::go_right;
			}
		}
		else if( disp.is_keyALT() || disp.is_keyALTGR() || cur_im_.depth() == 1 )
		{
			if( disp.wheel() > 0 )
			{
				view_action_ = ViewAction::go_up;
			}
			else
			{
				view_action_ = ViewAction::go_down;
			}
		}
		disp.set_wheel();
	}

	const int sx0 = selection( 0 );
	const int sy0 = selection( 1 );
	const int sz0 = selection( 2 );
	const int sx1 = selection( 3 );
	const int sy1 = selection( 4 );
	const int sz1 = selection( 5 );

	int key = 0;

	if( sx0 >= 0 && sy0 >= 0 && sz0 >= 0 && sx1 >= 0 && sy1 >= 0 && sz1 >= 0 )
	{
		lower_right_[0] = upper_left_[0] + sx1;
		lower_right_[1] = upper_left_[1] + sy1;
		lower_right_[2] = upper_left_[2] + sz1;
		upper_left_[0] += sx0;
		upper_left_[1] += sy0;
		upper_left_[2] += sz0;

		if( sx0 == sx1 && sy0 == sy1 && sz0 == sz1 )
		{
			view_action_ = ViewAction::reset_view;
		}
		resize_disp_ = true;
	}
	else
	{
		switch( key = disp.key() )
		{
			case cimg::keyHOME:
			case cimg::keyESC:
				view_action_ = ViewAction::reset_view;
				resize_disp_ = true;
				key			 = 0;
				break;

			case cimg::keyPADADD: view_action_ = ViewAction::zoom_in;
				center_view_				   = true;
				key							   = 0;
				break;

			case cimg::keyPADSUB: view_action_ = ViewAction::zoom_out;
				key							   = 0;
				break;

			case cimg::keyARROWLEFT:
			case cimg::keyPAD4: view_action_ = ViewAction::goto_next_frame;
				key							 = 0;
				break;

			case cimg::keyARROWRIGHT:
			case cimg::keyPAD6: view_action_ = ViewAction::goto_previous_frame;
				key							 = 0;
				break;
			case cimg::keyD:
				equalize_depth_image_ = !equalize_depth_image_;
				break;
			case cimg::keyH:
				histogram_equalize_ = !histogram_equalize_;
				break;
		}         // switch
	}
	return key;
} // OMViewerApp::getSelection

void OMViewerApp::performViewAction(cimg_library::CImgDisplay & disp)
{
	if( view_action_ == ViewAction::zoom_in )
	{
		const int mx = center_view_ ? disp.width() / 2 : disp.mouse_x();
		const int my = center_view_ ? disp.height() / 2 : disp.mouse_y();
		const int mX = mx * cur_im_.width()  + ( cur_im_.depth() > 1 ? cur_im_.depth() : 0 ) / disp.width();
		const int mY = my * cur_im_.height() + ( cur_im_.depth() > 1 ? cur_im_.depth() : 0 ) / disp.height();
		int X		 = XYZ_[0];
		int Y		 = XYZ_[1];
		int Z		 = XYZ_[2];

		if( mX < cur_im_.width() && mY < cur_im_.height() )
		{
			X = upper_left_[0] + mX * ( lower_right_[0] - upper_left_[0] + 1 ) / cur_im_.width();
			Y = upper_left_[1] + mY * ( lower_right_[1] - upper_left_[1] + 1 ) / cur_im_.height();
			Z = XYZ_[2];
		}

		if( mX < cur_im_.width() && mY >= cur_im_.height() )
		{
			X = upper_left_[0] + mX * ( lower_right_[0] - upper_left_[0] + 1 ) / cur_im_.width();
			Z = upper_left_[2] + mY * ( lower_right_[2] - upper_left_[2] + 1 ) / cur_im_.height();
			Y = XYZ_[1];
		}

		if( mX >= cur_im_.width() && mY < cur_im_.height() )
		{
			X = XYZ_[0];
			Y = upper_left_[1] + mY * ( lower_right_[1] - upper_left_[1] + 1 ) / cur_im_.height();
			Z = upper_left_[2] + mY * ( lower_right_[2] - upper_left_[2] + 1 ) / cur_im_.height();
		}

		if( lower_right_[0] - upper_left_[0] > 4 )
		{
			upper_left_[0]	= X - 7 * ( X - upper_left_[0] ) / 8;
			lower_right_[0] = X + 7 * ( lower_right_[0] - X ) / 8;
		}

		if( lower_right_[1] - upper_left_[1] > 4 )
		{
			upper_left_[1]	= X - 7 * ( X - upper_left_[1] ) / 8;
			lower_right_[1] = X + 7 * ( lower_right_[1] - X ) / 8;
		}

		if( lower_right_[2] - upper_left_[2] > 4 )
		{
			upper_left_[2]	= X - 7 * ( X - upper_left_[2] ) / 8;
			lower_right_[2] = X + 7 * ( lower_right_[2] - X ) / 8;
		}
	}

	if( view_action_ == ViewAction::zoom_out )
	{
		const int delta_x  = ( lower_right_[0] - upper_left_[0] ) / 8;
		const int delta_y  = ( lower_right_[1] - upper_left_[1] ) / 8;
		const int delta_z  = ( lower_right_[2] - upper_left_[2] ) / 8;
		const int ndelta_x = delta_x ? delta_x : ( cur_im_.width()  > 1 ? 1 : 0 );
		const int ndelta_y = delta_y ? delta_y : ( cur_im_.height() > 1 ? 1 : 0 );
		const int ndelta_z = delta_z ? delta_z : ( cur_im_.depth()  > 1 ? 1 : 0 );
		upper_left_[0]	-= ndelta_x;
		upper_left_[1]	-= ndelta_y;
		upper_left_[2]	-= ndelta_z;
		lower_right_[0] += ndelta_x;
		lower_right_[1] += ndelta_y;
		lower_right_[2] += ndelta_z;

		if( upper_left_[0] < 0 )
		{
			lower_right_[0] -= upper_left_[0];
			upper_left_[0]	 = 0;

			if( lower_right_[0] >= cur_im_.width() )
			{
				lower_right_[0] = cur_im_.width() - 1;
			}
		}

		if( upper_left_[1] < 0 )
		{
			lower_right_[1] -= upper_left_[1];
			upper_left_[1]	 = 0;

			if( lower_right_[1] >= cur_im_.height() )
			{
				lower_right_[1] = cur_im_.height() - 1;
			}
		}

		if( upper_left_[2] < 0 )
		{
			lower_right_[2] -= upper_left_[2];
			upper_left_[2]	 = 0;

			if( lower_right_[2] >= cur_im_.depth() )
			{
				lower_right_[2] = cur_im_.depth() - 1;
			}
		}

		if( lower_right_[0] >= cur_im_.width() )
		{
			upper_left_[0] -= ( lower_right_[0] - cur_im_.width() + 1 );
			lower_right_[0] = cur_im_.width() - 1;

			if( upper_left_[0] < 0 )
			{
				upper_left_[0] = 0;
			}
		}

		if( lower_right_[1] >= cur_im_.height() )
		{
			upper_left_[1] -= ( lower_right_[1] - cur_im_.height() + 1 );
			lower_right_[1] = cur_im_.height() - 1;

			if( upper_left_[1] < 0 )
			{
				upper_left_[1] = 0;
			}
		}

		if( lower_right_[2] >= cur_im_.depth() )
		{
			upper_left_[2] -= ( lower_right_[2] - cur_im_.depth() + 1 );
			lower_right_[2] = cur_im_.depth() - 1;

			if( upper_left_[2] < 0 )
			{
				upper_left_[2] = 0;
			}
		}
	}

	if( view_action_ == ViewAction::go_right )
	{
		const int delta	 = ( lower_right_[0] - upper_left_[0] ) / 5;
		const int ndelta = delta ? delta : ( cur_im_.width() > 1 ? 1 : 0 );

		if( lower_right_[0] + ndelta < cur_im_.width() )
		{
			upper_left_[0]	+= ndelta;
			lower_right_[0] += ndelta;
		}
		else
		{
			upper_left_[0] += ( cur_im_.width() - lower_right_[0] - 1 );
			lower_right_[0] = cur_im_.width() - 1;
		}
	}

	if( view_action_ == ViewAction::go_left )
	{
		const int delta	 = ( lower_right_[0] - upper_left_[0] ) / 5;
		const int ndelta = delta ? delta : ( cur_im_.width() > 1 ? 1 : 0 );

		if( upper_left_[0] - ndelta >= 0 )
		{
			upper_left_[0]	-= ndelta;
			lower_right_[0] -= ndelta;
		}
		else
		{
			lower_right_[0] -= upper_left_[0];
			upper_left_[0]	 = 0;
		}
	}

	if( view_action_ == ViewAction::go_up )
	{
		const int delta	 = ( lower_right_[1] - upper_left_[1] ) / 5;
		const int ndelta = delta ? delta : ( cur_im_.height() > 1 ? 1 : 0 );

		if( upper_left_[1] - ndelta >= 0 )
		{
			upper_left_[1]	-= ndelta;
			lower_right_[1] -= ndelta;
		}
		else
		{
			lower_right_[1] -= upper_left_[1];
			upper_left_[1]	 = 0;
		}
	}

	if( view_action_ == ViewAction::go_down )
	{
		const int delta	 = ( lower_right_[1] - upper_left_[1] ) / 5;
		const int ndelta = delta ? delta : ( cur_im_.height() > 1 ? 1 : 0 );

		if( lower_right_[1] + ndelta < cur_im_.height() )
		{
			upper_left_[1]	+= ndelta;
			lower_right_[1] += ndelta;
		}
		else
		{
			upper_left_[1] += ( cur_im_.height() - 1 - lower_right_[1] );
			lower_right_[1] = cur_im_.height() - 1;
		}
	}

	if( view_action_ == ViewAction::goto_next_frame )
	{
		cur_im_ = im_list_.previousImage();
		disp.set_title( im_list_.getCurrentFileName().c_str() );
	}

	if( view_action_ == ViewAction::goto_previous_frame )
	{
		cur_im_ = im_list_.nextImage();
		disp.set_title( im_list_.getCurrentFileName().c_str() );
	}
	if( view_action_ == ViewAction::reset_view )
	{
		resetView(disp);
	}
	view_action_ = ViewAction::no_action;
}

/*
 *	Modified from CImg<T>._display() since it already has most of the logic
 * for resizing etc.
 */
int OMViewerApp::display( CImgDisplay& disp )
{
	int key;

	old_w_			= old_h_ = key = 0;
	upper_left_[0]	= upper_left_[1] = upper_left_[2] = 0;
	lower_right_[0] = cur_im_.width() - 1;
	lower_right_[1] = cur_im_.height() - 1;
	lower_right_[2] = cur_im_.depth() - 1;
	XYZ_[0]			= ( lower_right_[0] - upper_left_[0] ) / 2;
	XYZ_[1]			= ( lower_right_[1] - upper_left_[1] ) / 2;
	XYZ_[2]			= ( lower_right_[2] - upper_left_[2] ) / 2;
	resetView(disp);
	disp.show().flush();

	CImg<float> zoom;

	while( !disp.is_closed() && disp.key() != cimg::keyQ)
	{
		cur_view_im_ = cur_im_;
		if( upper_left_[0]  == 0 &&
			upper_left_[1]  == 0 &&
			upper_left_[2]  == 0 &&
			lower_right_[0] == cur_im_.width() - 1 &&
			lower_right_[1] == cur_im_.height() - 1 &&
			lower_right_[2] == cur_im_.depth() - 1 )
		{
			if( cur_im_.is_empty() )
			{
				zoom.assign( 1, 1, 1, 1, 0 );
			}
			else
			{
				zoom.assign();
			}
		}
		else
		{
			zoom = cur_view_im_.get_crop( upper_left_[0],
									 upper_left_[1],
									 upper_left_[2],
									 lower_right_[0],
									 lower_right_[1],
									 lower_right_[2] );
		}

		const unsigned int dx = lower_right_[0] - upper_left_[0] + 1;
		const unsigned int dy = lower_right_[1] - upper_left_[1] + 1;
		const unsigned int dz = lower_right_[2] - upper_left_[2] + 1;
		const unsigned int tw = dx + ( dz > 1 ? dz : 0 );
		const unsigned int th = dy + ( dz > 1 ? dz : 0 );

		if( !cur_im_.is_empty() && !disp.is_fullscreen() && resize_disp_ )
		{
			resizeView( disp, tw, th );
			resize_disp_ = false;
		}
		old_w_ = tw;
		old_h_ = th;

		if(equalize_depth_image_)
		{
			float new_val = cur_view_im_.max() + 10;
			cimg_forXY(cur_view_im_, x, y)
			{
				if (cur_view_im_(x,y) == 0) {
					cur_view_im_(x,y) = new_val;
				}
			}
		}
		if(histogram_equalize_)
		{
			const int numBins = 100;
			cur_view_im_ = cur_view_im_.equalize(numBins);
		}

		const CImg<float>& visu = zoom ? zoom : cur_view_im_ ;

		if( cur_im_.width() > 1  && visu.width() == 1 )
		{
			disp.set_title( "%s | x=%u", disp._title, upper_left_[0] );
		}

		if( cur_im_.height() > 1 && visu.height() == 1 )
		{
			disp.set_title( "%s | y=%u", disp._title, upper_left_[1] );
		}

		if( cur_im_.depth() > 1 && visu.depth() == 1 )
		{
			disp.set_title( "%s | z=%u", disp._title, upper_left_[2] );
		}

		key = getSelection( visu, disp );
		performViewAction( disp );
		disp.wait( 100 );
	}
	disp.set_key( key );
} // OMViewerApp::display
} // namespace
