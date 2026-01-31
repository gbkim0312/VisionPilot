#include "pangolin_viewer_adapter_impl.hpp"
#include "gaia_log.hpp"

namespace vp::adapter::out
{
PangolinViewerAdapterImpl::PangolinViewerAdapterImpl(const config::VslamViewerConfig &config)
    : config_{config}, pose_history_{}
{
    LOG_TRA("");
}

PangolinViewerAdapterImpl::~PangolinViewerAdapterImpl()
{
    this->stop();
    LOG_TRA("");
}

bool PangolinViewerAdapterImpl::start()
{
    LOG_TRA("Starting Pangolin Viewer...");
    return true;
}
bool PangolinViewerAdapterImpl::stop()
{
    LOG_TRA("Stopping Pangolin Viewer...");
    return true;
}
void PangolinViewerAdapterImpl::render(const domain::model::Pose &pose, const domain::model::ImagePacket & /* frame */)
{
    LOG_TRA("Rendering frame in Pangolin Viewer...");
}

} // namespace vp::adapter::out

// #include <Eigen/Dense>
// #include <Eigen/Geometry>

// #include "gaia_log.hpp"
// #include "pangolin_viewer_adapter_impl.hpp"
// #include <opencv2/core.hpp>
// #include <opencv2/imgproc.hpp>
// #include <pangolin/gl/glfont.h>

// namespace vp::adapter::out
// {

// // variant 방문을 위한 헬퍼
// template <class... Ts>
// struct overloaded : Ts...
// {
//     using Ts::operator()...;
// };
// template <class... Ts>
// overloaded(Ts...) -> overloaded<Ts...>;

// PangolinViewerAdapterImpl::PangolinViewerAdapterImpl(const config::VslamViewerConfig &config)
//     : config_{config}, s_cam_{nullptr}, pose_history_{}
// {
//     LOG_TRA("");
// }

// PangolinViewerAdapterImpl::~PangolinViewerAdapterImpl()
// {
//     this->stop();
//     LOG_TRA("");
// }

// bool PangolinViewerAdapterImpl::start()
// {
//     LOG_INF("Starting Pangolin Viewer...");

//     // 1. 창 생성 및 바인딩
//     pangolin::CreateWindowAndBind(window_name_, 1024, 768);
//     glEnable(GL_DEPTH_TEST);
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//     // 2. 3D 카메라 투영/모델뷰 행렬 설정
//     s_cam_ = std::make_unique<pangolin::OpenGlRenderState>(
//         pangolin::ProjectionMatrix(1024, 768, 420, 420, 512, 384, 0.1, 1000),
//         pangolin::ModelViewLookAt(-2, 2, -5, 0, 0, 0, pangolin::AxisY));

//     // 3. 인터렉티브 뷰포트 설정
//     d_cam_ = &pangolin::CreateDisplay()
//                   .SetBounds(0.0, 1.0, 0.0, 1.0)
//                   .SetHandler(new pangolin::Handler3D(*s_cam_));

//     is_running_ = true;
//     return true;
// }

// bool PangolinViewerAdapterImpl::stop()
// {
//     LOG_INF("Stopping Pangolin Viewer...");
//     is_running_ = false;
//     return true;
// }

// void PangolinViewerAdapterImpl::render(const domain::model::Pose &pose, const domain::model::ImagePacket & /* frame */)
// {
//     if (!is_running_ || pangolin::ShouldQuit())
//     {
//         return;
//     }

//     if (!pose.is_lost)
//     {
//         pose_history_.push_back(pose);
//         if (pose_history_.size() > max_history_size_)
//         {
//             pose_history_.erase(pose_history_.begin());
//         }
//     }

//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//     glClearColor(0.15F, 0.15F, 0.15F, 1.0F);

//     d_cam_->Activate(*s_cam_);

//     // 에러 해결: glDraw_PlaneGrid가 없는 경우 직접 그리기 루틴으로 대체
//     glColor3f(0.3F, 0.3F, 0.3F);
//     // pangolin::glDraw_PlaneGrid 대신 간단한 그리드 그리기 (안전한 방식)
//     glBegin(GL_LINES);
//     for (int i = -10; i <= 10; ++i)
//     {
//         glVertex3f(static_cast<float>(i), 0, -10.0F);
//         glVertex3f(static_cast<float>(i), 0, 10.0F);
//         glVertex3f(-10.0F, 0, static_cast<float>(i));
//         glVertex3f(10.0F, 0, static_cast<float>(i));
//     }
//     glEnd();

//     this->drawTrajectory();

//     if (!pose.is_lost)
//     {
//         // Eigen 네임스페이스와 타입 에러 해결
//         Eigen::Quaterniond q(pose.qw, pose.qx, pose.qy, pose.qz);
//         Eigen::Vector3d t(pose.x, pose.y, pose.z);

//         Eigen::Matrix4d Twc = Eigen::Matrix4d::Identity();
//         Twc.block<3, 3>(0, 0) = q.toRotationMatrix();
//         Twc.block<3, 1>(0, 3) = t;

//         glPushMatrix();
//         // Clang-Tidy가 지적한 comma operator 에러 방지를 위해 명시적 데이터 전달
//         const std::vector<double> mat_vec{
//             Twc(0, 0), Twc(1, 0), Twc(2, 0), Twc(3, 0),
//             Twc(0, 1), Twc(1, 1), Twc(2, 1), Twc(3, 1),
//             Twc(0, 2), Twc(1, 2), Twc(2, 2), Twc(3, 2),
//             Twc(0, 3), Twc(1, 3), Twc(2, 3), Twc(3, 3)};
//         glMultMatrixd(mat_vec.data());

//         pangolin::glDrawAxis(0.6);
//         glPopMatrix();
//     }

//     if (pose.is_lost)
//     {
//         LOG_WRN("VSLAM Tracking Lost...");
//     }

//     pangolin::FinishFrame();
// }

// void PangolinViewerAdapterImpl::drawTrajectory()
// {
//     if (pose_history_.size() < 2)
//     {
//         return;
//     }

//     glLineWidth(2.5);
//     glBegin(GL_LINE_STRIP);
//     glColor3f(0.0F, 1.0F, 1.0F); // 궤적 색상: 사이언(Cyan)

//     for (const auto &p : pose_history_)
//     {
//         glVertex3d(p.x, p.y, p.z);
//     }
//     glEnd();
// }

// } // namespace vp::adapter::out