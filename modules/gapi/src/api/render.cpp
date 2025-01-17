#include "precomp.hpp"

#include <stdexcept>

#include <opencv2/imgproc.hpp>
#include <opencv2/gapi/render/render.hpp>
#include <opencv2/gapi/own/assert.hpp>

#include "api/render_priv.hpp"

void cv::gapi::wip::draw::render(cv::Mat& bgr,
                                 const cv::gapi::wip::draw::Prims& prims,
                                 cv::GCompileArgs&& args)
{
    cv::GMat in;
    cv::GArray<cv::gapi::wip::draw::Prim> arr;

    cv::GComputation comp(cv::GIn(in, arr),
                          cv::GOut(cv::gapi::wip::draw::render3ch(in, arr)));
    comp.apply(cv::gin(bgr, prims), cv::gout(bgr), std::move(args));
}

void cv::gapi::wip::draw::render(cv::Mat& y_plane,
                                 cv::Mat& uv_plane,
                                 const Prims& prims,
                                 cv::GCompileArgs&& args)
{
    cv::GMat y_in, uv_in, y_out, uv_out;
    cv::GArray<cv::gapi::wip::draw::Prim> arr;
    std::tie(y_out, uv_out) = cv::gapi::wip::draw::renderNV12(y_in, uv_in, arr);

    cv::GComputation comp(cv::GIn(y_in, uv_in, arr), cv::GOut(y_out, uv_out));
    comp.apply(cv::gin(y_plane, uv_plane, prims),
               cv::gout(y_plane, uv_plane), std::move(args));
}

void cv::gapi::wip::draw::cvtYUVToNV12(const cv::Mat& yuv,
                                       cv::Mat& y,
                                       cv::Mat& uv)
{
    GAPI_Assert(yuv.size().width  % 2 == 0);
    GAPI_Assert(yuv.size().height % 2 == 0);

    std::vector<cv::Mat> chs(3);
    cv::split(yuv, chs);
    y = chs[0];
    cv::merge(std::vector<cv::Mat>{chs[1], chs[2]}, uv);
    cv::resize(uv, uv, uv.size() / 2, cv::INTER_LINEAR);
}

void cv::gapi::wip::draw::cvtNV12ToYUV(const cv::Mat& y,
                                       const cv::Mat& uv,
                                       cv::Mat& yuv)
{
    cv::Mat upsample_uv;
    cv::resize(uv, upsample_uv, uv.size() * 2, cv::INTER_LINEAR);
    cv::merge(std::vector<cv::Mat>{y, upsample_uv}, yuv);
}

namespace cv
{
namespace detail
{
    template<> struct CompileArgTag<cv::gapi::wip::draw::freetype_font>
    {
        static const char* tag() { return "gapi.freetype_font"; }
    };

} // namespace detail

GMat cv::gapi::wip::draw::render3ch(const GMat& src,
                                    const GArray<cv::gapi::wip::draw::Prim>& prims)
{
    return cv::gapi::wip::draw::GRenderBGR::on(src, prims);
}

std::tuple<GMat, GMat> cv::gapi::wip::draw::renderNV12(const GMat& y,
                                                       const GMat& uv,
                                                       const GArray<cv::gapi::wip::draw::Prim>& prims)
{
    return cv::gapi::wip::draw::GRenderNV12::on(y, uv, prims);
}

} // namespace cv
