#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifdef _DEBUG
	#pragma comment(lib, "opencv_core246d.lib")
	#pragma comment(lib, "opencv_imgproc246d.lib")
	#pragma comment(lib, "opencv_highgui246d.lib")
#else
	#pragma comment(lib, "opencv_core246.lib")
	#pragma comment(lib, "opencv_imgproc246.lib")
	#pragma comment(lib, "opencv_highgui246.lib")
#endif

class ChromaKey : public cv::ParallelLoopBody
{
    public:
        ChromaKey(const cv::Mat &fg, const cv::Mat &bg, cv::Mat &dst, const cv::Scalar &color = cv::Scalar(255,0,255), const int &threshold = 30)
			: ParallelLoopBody(), fg_(fg), bg_(bg), dst_(dst), color_(color), threshold_(threshold)
        {
        }

        void operator ()(const cv::Range& range) const
        {
            for (int y = range.start; y < range.end; ++y) {
				const uchar *fg_p = fg_.ptr<uchar>(y);
				const uchar *bg_p = bg_.ptr<uchar>(y);
				uchar *dst_p       = dst_.ptr<uchar>(y);

				for (int x = 0; x < fg_.cols; ++x) {
					int i = x * fg_.channels();

					int ch_r = color_.val[2];
					int ch_g = color_.val[1];
					int ch_b = color_.val[0];

					int fg_r = fg_p[i + 2];
					int fg_g = fg_p[i + 1];
					int fg_b = fg_p[i + 0];

					int bg_r = bg_p[i + 2];
					int bg_g = bg_p[i + 1];
					int bg_b = bg_p[i + 0];

					int dr = fg_r - ch_r;
					int dg = fg_g - ch_g;
					int db = fg_b - ch_b;
					int dd = dr * dr + dg * dg + db * db;

					if (dd <= threshold_ * threshold_) {
						dst_p[i + 2] = bg_r;
						dst_p[i + 1] = bg_g;
						dst_p[i + 0] = bg_b;
					}
					else {
						dst_p[i + 2] = fg_r;
						dst_p[i + 1] = fg_g;
						dst_p[i + 0] = fg_b;
					}
				}
            }
        }

private:
        const cv::Mat &fg_;
        const cv::Mat &bg_;
        cv::Mat &dst_;
		const cv::Scalar color_;
		const int threshold_;
};

void draw_text(cv::Mat &canvas, const char *str, int x, int y, float scale = 1.0)
{
	int w = 2;
	for(int dy = -w; dy <= w; ++dy) {
		for (int dx = -w; dx <= w; ++dx) {
			cv::putText(canvas, str, cv::Point(x+dx,y+dy), cv::FONT_HERSHEY_SIMPLEX, scale, cv::Scalar(0,0,0), 2, CV_AA);
		}
	}
	cv::putText(canvas, str, cv::Point(x,y), cv::FONT_HERSHEY_SIMPLEX, scale, cv::Scalar(255,255,255), 2, CV_AA);
}

int main(int argc, char* argv[])
{
	cv::Mat bg_img = cv::imread("../img/bg.jpg");
	cv::Mat result_img(bg_img.size(), CV_8UC3);

	cv::Mat fg_img(bg_img.size(), CV_8UC3);
	fg_img = CV_RGB(255,0,255);

	draw_text(fg_img, "test string", 100, 100);


    ChromaKey body(fg_img, bg_img, result_img);
    cv::parallel_for_(cv::Range(0, fg_img.rows), body);

	cv::imshow("fg_img", fg_img);
	cv::imshow("bg_img", bg_img);
	cv::imshow("result_img", result_img);

	int c = cv::waitKey(0);

	return 0;
}

