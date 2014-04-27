#include "ImageList.hpp"

namespace omviewer {
enum class ViewAction
{
	zoom_in, zoom_out, go_left, go_right, go_down, go_up, goto_next_frame, goto_previous_frame, go_in_center
};

class OMViewerApp {
public:

	OMViewerApp( const std::string& image_dir ):im_list_( image_dir ) {}

	void run();

private:

	int	 display( cimg_library::CImgDisplay& disp );
	void resetView( cimg_library::CImgDisplay& disp );
	void resizeView( cimg_library::CImgDisplay& disp,
					 unsigned int				tw,
					 unsigned int				th );
	int	 getSelection( const cimg_library::CImg<float>& im,
					   cimg_library::CImgDisplay	  & disp );

	ImageList im_list_;
	cimg_library::CImg<float> cur_im_;

	unsigned int XYZ_[3];
	int upper_left_[3];
	int lower_right_[3];
	int old_w_;
	int old_h_;
	ViewAction view_action_;
	bool reset_view_;
	bool center_view_;
	bool resize_disp_;
};
} // namespace
