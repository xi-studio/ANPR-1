#include "ANPR.h"

#include <string>
#include <algorithm>

#include <boost/format.hpp>
#include <boost/regex.hpp>

#include <tesseract/baseapi.h>

#include "help_alg.h"
#include "help_opencv.h"


std::string ANPR::recognize_number_plate(const cv::Mat &number_plate_image)
{
  cv::Mat denoised_image;
  cv::medianBlur(number_plate_image, denoised_image, 3);

  int center_x = denoised_image.cols / 2;
  int center_y = denoised_image.rows / 2;
  int half_width = denoised_image.cols / 10 / 2;
  int half_height = denoised_image.rows / 10 / 2;
  cv::Mat search_area = denoised_image(cv::Range(center_y - half_height,
                                                 center_y + half_height),
                                       cv::Range(center_x - half_width,
                                                 center_x + half_width));
  auto minmax_color = std::minmax_element(search_area.begin<cv::Vec<unsigned char, 3>>(),
                                          search_area.end<cv::Vec<unsigned char, 3>>(),
                                          [](const cv::Vec<unsigned char, 3> &a,
                                             const cv::Vec<unsigned char, 3> &b)
  {
    return cv::norm(a) < cv::norm(b);
  });

  cv::Mat balanced_image;
  set_white_balance<3>(denoised_image, balanced_image,
                       *minmax_color.first, *minmax_color.second);

  cv::Mat grayscale_image;
  cv::cvtColor(balanced_image, grayscale_image, CV_RGB2GRAY);

  cv::Mat threshold_image;
  cv::threshold(grayscale_image, threshold_image, 80.0, 255.0, CV_THRESH_BINARY);

  std::vector<cv::Point> area;
  int pos = std::distance(search_area.begin<cv::Vec<unsigned char, 3>>(),
                          minmax_color.second);
  cv::Point white_color_pos;
  white_color_pos.x = center_x - half_width + (pos % search_area.cols);
  white_color_pos.y = center_y - half_height + (pos / search_area.cols);
  std::cout << white_color_pos << std::endl;
  find_filled_area(threshold_image, area, white_color_pos, 255);

  cv::Mat area_image = cv::Mat::zeros(threshold_image.size(), CV_8UC1);
  draw_area(area_image, area, 255);
  cv::imshow("area_image", area_image);

//  std::vector<std::vector<cv::Point> > areas = find_filled_areas(threshold_image.clone(), 255);

//  // remove too wide or too small areas
//  areas.erase(std::remove_if(areas.begin(), areas.end(),
//                             [threshold_image](const std::vector<cv::Point> &area)
//  {
//    cv::Rect area_bound = cv::boundingRect(area);

//    const double k0 = 1.1;
//    const double k1 = 0.25;

//    double ratio = (double)area_bound.width / area_bound.height;

//    return ratio > k0 || ratio < k1 ||
////           (double)area.size() / threshold_image.total() < 0.006 ||
//           area_bound.height == threshold_image.size().height ||
//           area_bound.x == 0 || area_bound.y == 0 ||
//           area_bound.x + area_bound.width == threshold_image.size().width ||
//           area_bound.y + area_bound.height == threshold_image.size().height;
//  }), areas.end());

//  int max_height = 0;
//  for (auto &area: areas) {
//    cv::Rect area_bound = cv::boundingRect(area);
//    max_height = std::max(max_height, area_bound.height);
//  }

//  double height_threshold_coef = 0.5;
//  int height_threshold = (int)(max_height * height_threshold_coef);
//  areas.erase(std::remove_if(areas.begin(), areas.end(),
//                             [height_threshold](const std::vector<cv::Point> &area)
//  {
//    cv::Rect area_bound = cv::boundingRect(area);
//    return area_bound.height < height_threshold;
//  }), areas.end());

//  std::string number_plate_text;
//  if (areas.size() > 0)
//  {
//    cv::Mat tess_image = cv::Mat::zeros(threshold_image.size(), threshold_image.type());
//    for (auto &area: areas)
//      draw_area(tess_image, area, 255);

//    cv::imshow("tess_image", tess_image);

//    tesseract::TessBaseAPI tess_api;
//    tess_api.Init("tessdata", "eng");
//    tess_api.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
//    tess_api.SetVariable("tessedit_char_whitelist", "ABCEHKMOPTXY1234567890");

//    tess_api.SetImage(tess_image.ptr(),
//                      tess_image.size().width,
//                      tess_image.size().height,
//                      tess_image.elemSize(),
//                      tess_image.step1());
//    char *text = tess_api.GetUTF8Text();
//    number_plate_text = text;
//    delete[] text;

//    number_plate_text.erase(std::remove_if(number_plate_text.begin(),
//                                           number_plate_text.end(),
//                                           isspace),
//                            number_plate_text.end());


//    boost::match_results<std::string::iterator> what;
//    if (boost::regex_search(number_plate_text.begin(),
//                            number_plate_text.end(),
//                            what,
//                            boost::regex("[ABCEHKMOPTXY0][[:digit:]]{3}[ABCEHKMOPTXY0]{2}[[:digit:]]{2,3}")))
//    {
//      int zero_test_indexes[] = {0, 4, 5};
//      for (auto index: zero_test_indexes)
//        if (*(what[0].first + index) == '0')
//          *(what[0].first + index) = 'O';

//      number_plate_text = std::string(what[0].first, what[0].second);
//    }
//  }

//  return number_plate_text;
  return "";
}
