#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <CImg.h>

namespace omviewer
{

class ImageList
{
public:
	ImageList(const std::string& dir_name = "./");
	cimg_library::CImg<float> nextImage();
	cimg_library::CImg<float> previousImage();
	cimg_library::CImg<float> getImage(int i = -1) const;
	std::string getCurrentFileName() const;

private:
	int cur_i_;
	std::vector<std::string> file_names_;
	std::string dir_name_;

	const std::vector<std::string> supported_image_types_ = {".png", ".jpg", ".ppm", ".jpeg", ".gif", ".tiff"}; // c++11 required

	void readFiles(const std::string & dir_name);
};

} // namespace