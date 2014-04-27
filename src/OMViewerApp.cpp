#include "OMViewerApp.hpp"

#include <CImg.h>

using namespace cimg_library;

namespace omviewer
{
	void OMViewerApp::run()
	{
		CImg<float> im = im_list_.getImage(0);
		CImgDisplay main_disp(im);
		main_disp.set_title(im_list_.getCurrentFileName().c_str());
		display(im, main_disp);
	}

	/*
	 *	Modified from CImg<T>._display() since it already has most of the logic for resizing etc.
	 */
	int OMViewerApp::display(CImg<float>& im, CImgDisplay & disp)
	{
		bool normalize = true;
			unsigned int oldw = 0, oldh = 0, _XYZ[3], key = 0;
			int x0 = 0, y0 = 0, z0 = 0, x1 = im.width()-1, y1 = im.height()-1, z1 = im.depth()-1;
			disp.show().flush();

			CImg<float> zoom;

			for (bool reset_view = true, resize_disp = false, is_first_select = true;  !disp.is_closed(); )
			{
				if (reset_view)
				{
					_XYZ[0] = (x0 + x1)/2;
					_XYZ[1] = (y0 + y1)/2;
					_XYZ[2] = (z0 + z1)/2;
					x0 = 0; y0 = 0; z0 = 0;
					x1 = im.width()-1;
					y1 = im.height()-1;
					z1 = im.depth()-1;
					oldw = disp.width();
					oldh = disp.height();
					reset_view = false;
				}
				if (!x0 && !y0 && !z0 && x1 == im.width()-1 && y1 == im.height()-1 && z1 == im.depth()-1)
				{
					if (im.is_empty())
					{
						zoom.assign(1,1,1,1,0);
					}
					else
					{
						zoom.assign();
					}
				}
				else
				{
					zoom = im.get_crop(x0,y0,z0,x1,y1,z1);
				}

				const unsigned int dx = 1 + x1 - x0;
				const unsigned int dy = 1 + y1 - y0;
				const unsigned int dz = 1 + z1 - z0,
				tw = dx + (dz>1?dz:0), th = dy + ( dz > 1 ? dz : 0 );
				if (!im.is_empty() && !disp.is_fullscreen() && resize_disp)
				{
					const unsigned int ttw = tw*disp.width()/oldw;
					const unsigned int tth = th*disp.height()/oldh;
					const unsigned int dM = cimg::max(ttw,tth), diM = (unsigned int)cimg::max(disp.width(),disp.height());
					const unsigned int imgw = cimg::max(16U,ttw*diM/dM);
					const unsigned int imgh = cimg::max(16U,tth*diM/dM);
					disp.set_fullscreen(false).resize(cimg_fitscreen(imgw,imgh,1),false);
					resize_disp = false;
				}
				oldw = tw;
				oldh = th;

				bool go_up = false;
				bool go_down = false;
				bool go_left = false;
				bool go_right = false;
				bool go_inc = false;
				bool go_dec = false;
				bool go_in = false;
				bool go_out = false;
				bool go_in_center = false;

				const CImg<float>& visu = zoom ? zoom : im;

				if (im._width>1 && visu._width==1) disp.set_title("%s | x=%u",disp._title,x0);
				if (im._height>1 && visu._height==1) disp.set_title("%s | y=%u",disp._title,y0);
				if (im._depth>1 && visu._depth==1) disp.set_title("%s | z=%u",disp._title,z0);

				if (!is_first_select)
				{
					_XYZ[0] = (x1-x0)/2;
					_XYZ[1] = (y1-y0)/2;
					_XYZ[2] = (z1-z0)/2;
				}
				const CImg<cimg::last<float,int>::type> selection = visu._get_select(disp,0,2,_XYZ,x0,y0,z0,is_first_select,im._depth>1);
				is_first_select = false;

				if (disp.wheel())
				{
					if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT())
					{
						go_out = !(go_in = disp.wheel()>0); go_in_center = false;
					}
					else if (disp.is_keySHIFTLEFT() || disp.is_keySHIFTRIGHT())
					{
						go_right = !(go_left = disp.wheel()>0);
					}
					else if (disp.is_keyALT() || disp.is_keyALTGR() || im._depth==1)
					{
						go_down = !(go_up = disp.wheel()>0);
					}
					disp.set_wheel();
				}

				const int sx0 = selection(0);
				const int sy0 = selection(1);
				const int sz0 = selection(2);
				const int sx1 = selection(3);
				const int sy1 = selection(4);
				const int sz1 = selection(5);
				if (sx0>=0 && sy0>=0 && sz0>=0 && sx1>=0 && sy1>=0 && sz1>=0)
				{
					x1 = x0 + sx1;
					y1 = y0 + sy1;
					z1 = z0 + sz1;
					x0+=sx0;
					y0+=sy0;
					z0+=sz0;
					if (sx0==sx1 && sy0==sy1 && sz0==sz1)
					{
						reset_view = true;
					}
					resize_disp = true;
				}
				else switch (key = disp.key())
				{

					case cimg::keyHOME : case cimg::keyESC : reset_view = resize_disp = true; key = 0; break;
					case cimg::keyPADADD : go_in = true; go_in_center = true; key = 0; break;
					case cimg::keyPADSUB : go_out = true; key = 0; break;
					case cimg::keyARROWLEFT : case cimg::keyPAD4: go_left = true; key = 0; break;
					case cimg::keyARROWRIGHT : case cimg::keyPAD6: go_right = true; key = 0; break;
//					case cimg::keyARROWUP : case cimg::keyPAD8: go_up = true; key = 0; break;
//					case cimg::keyARROWDOWN : case cimg::keyPAD2: go_down = true; key = 0; break;
					case cimg::keyPAD7 : go_up = go_left = true; key = 0; break;
					case cimg::keyPAD9 : go_up = go_right = true; key = 0; break;
					case cimg::keyPAD1 : go_down = go_left = true; key = 0; break;
					case cimg::keyPAD3 : go_down = go_right = true; key = 0; break;
					case cimg::keyPAGEUP : go_inc = true; key = 0; break;
					case cimg::keyPAGEDOWN : go_dec = true; key = 0; break;
				}
				if (go_in) {
					const int
					mx = go_in_center?disp.width()/2:disp.mouse_x(),
					my = go_in_center?disp.height()/2:disp.mouse_y(),
					mX = mx*(im._width+(im._depth>1?im._depth:0))/disp.width(),
					mY = my*(im._height+(im._depth>1?im._depth:0))/disp.height();
					int X = _XYZ[0], Y = _XYZ[1], Z = _XYZ[2];
					if (mX<im.width() && mY<im.height())  { X = x0 + mX*(1+x1-x0)/im._width; Y = y0 + mY*(1+y1-y0)/im._height; Z = _XYZ[2]; }
					if (mX<im.width() && mY>=im.height()) { X = x0 + mX*(1+x1-x0)/im._width; Z = z0 + (mY-im._height)*(1+z1-z0)/im._depth; Y = _XYZ[1]; }
					if (mX>=im.width() && mY<im.height()) { Y = y0 + mY*(1+y1-y0)/im._height; Z = z0 + (mX-im._width)*(1+z1-z0)/im._depth; X = _XYZ[0]; }
					if (x1-x0>4) { x0 = X - 7*(X-x0)/8; x1 = X + 7*(x1-X)/8; }
					if (y1-y0>4) { y0 = Y - 7*(Y-y0)/8; y1 = Y + 7*(y1-Y)/8; }
					if (z1-z0>4) { z0 = Z - 7*(Z-z0)/8; z1 = Z + 7*(z1-Z)/8; }
				}
				if (go_out) {
					const int
					delta_x = (x1-x0)/8, delta_y = (y1-y0)/8, delta_z = (z1-z0)/8,
					ndelta_x = delta_x?delta_x:(im._width>1?1:0),
					ndelta_y = delta_y?delta_y:(im._height>1?1:0),
					ndelta_z = delta_z?delta_z:(im._depth>1?1:0);
					x0-=ndelta_x; y0-=ndelta_y; z0-=ndelta_z;
					x1+=ndelta_x; y1+=ndelta_y; z1+=ndelta_z;
					if (x0<0) { x1-=x0; x0 = 0; if (x1>=im.width()) x1 = im.width() - 1; }
					if (y0<0) { y1-=y0; y0 = 0; if (y1>=im.height()) y1 = im.height() - 1; }
					if (z0<0) { z1-=z0; z0 = 0; if (z1>=im.depth()) z1 = im.depth() - 1; }
					if (x1>=im.width()) { x0-=(x1-im.width()+1); x1 = im.width()-1; if (x0<0) x0 = 0; }
					if (y1>=im.height()) { y0-=(y1-im.height()+1); y1 = im.height()-1; if (y0<0) y0 = 0; }
					if (z1>=im.depth()) { z0-=(z1-im.depth()+1); z1 = im.depth()-1; if (z0<0) z0 = 0; }
				}
				if (go_left) {
					im = im_list_.previousImage();
					disp.set_title(im_list_.getCurrentFileName().c_str());
				}
				if (go_right) {
					im = im_list_.nextImage();
					disp.set_title(im_list_.getCurrentFileName().c_str());

				}
//				if (go_up) {
//					const int delta = (y1-y0)/5, ndelta = delta?delta:(im._height>1?1:0);
//					if (y0-ndelta>=0) { y0-=ndelta; y1-=ndelta; }
//					else { y1-=y0; y0 = 0; }
//				}
//				if (go_down) {
//					const int delta = (y1-y0)/5, ndelta = delta?delta:(im._height>1?1:0);
//					if (y1+ndelta<im.height()) { y0+=ndelta; y1+=ndelta; }
//					else { y0+=(im.height()-1-y1); y1 = im.height()-1; }
//				}
				if (go_inc) {
					const int delta = (z1-z0)/5, ndelta = delta?delta:(im._depth>1?1:0);
					if (z0-ndelta>=0) { z0-=ndelta; z1-=ndelta; }
					else { z1-=z0; z0 = 0; }
				}
				if (go_dec) {
					const int delta = (z1-z0)/5, ndelta = delta?delta:(im._depth>1?1:0);
					if (z1+ndelta<im.depth()) { z0+=ndelta; z1+=ndelta; }
					else { z0+=(im.depth()-1-z1); z1 = im.depth()-1; }
				}

				disp.wait(100);
			}
			disp.set_key(key);
		}
} // namespace