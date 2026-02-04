#include "pangolin_viewer_adapter_impl.hpp"
#include "gaia_log.hpp"
#include <Eigen/Dense>
#include <GL/gl.h>

namespace vp::adapter::out
{
PangolinViewerAdapterImpl::PangolinViewerAdapterImpl(const config::ViewerConfig &config)
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
    if (is_running_)
    {
        return true;
    }

    is_running_ = true;

    render_thread_ = std::thread([this]()
                                 {
        pangolin::CreateWindowAndBind(window_name_, 1024, 768);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        s_cam_ = std::make_unique<pangolin::OpenGlRenderState>(
            pangolin::ProjectionMatrix(1024, 768, 500, 500, 512, 389, 0.1, 1000),
            pangolin::ModelViewLookAt(
                0.0, -5.0, -10.0,
                0.0, 0.0, 0.0,
                pangolin::AxisY));

        d_cam_ = &pangolin::CreateDisplay()
                      .SetBounds(0.0, 1.0, 0.0, 1.0)
                      .SetHandler(new pangolin::Handler3D(*s_cam_));

        while (is_running_ && !pangolin::ShouldQuit())
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT: opnencv
            glClearColor(0.15F, 0.15F, 0.15F, 1.0F);

            d_cam_->Activate(*s_cam_);

            drawGrid();

            {
                std::lock_guard<std::mutex> lock(data_mutex_);
                if (has_pose_)
                {
                    drawTrajectory();
                    drawCurrentPose(latest_pose_);
                }
            }

            pangolin::FinishFrame();
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60Hz
        } });

    return true;
}

bool PangolinViewerAdapterImpl::stop()
{
    if (!is_running_)
    {
        return true;
    }

    is_running_ = false;

    if (render_thread_.joinable())
    {
        render_thread_.join();
    }

    return true;
}

void PangolinViewerAdapterImpl::render(
    const domain::model::Pose &pose,
    const std::vector<domain::model::Detection> &,
    const domain::model::ImagePacket &)
{
    if (!is_running_)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(data_mutex_);

    if (!pose.is_lost)
    {
        latest_pose_ = pose;
        has_pose_ = true;

        pose_history_.push_back(pose);
        if (pose_history_.size() > max_history_size_)
        {
            pose_history_.erase(pose_history_.begin());
        }
    }
}

void PangolinViewerAdapterImpl::drawTrajectory()
{
    if (pose_history_.size() < 2)
    {
        return;
    }

    glLineWidth(2.5F);
    glColor3f(0.0F, 1.0F, 1.0F);

    glBegin(GL_LINE_STRIP);
    for (const auto &p : pose_history_)
    {
        glVertex3d(p.x, p.y, p.z);
    }
    glEnd();
}

void PangolinViewerAdapterImpl::drawCurrentPose(const domain::model::Pose &pose)
{
    Eigen::Quaterniond q(pose.qw, pose.qx, pose.qy, pose.qz);
    Eigen::Vector3d t(pose.x, pose.y, pose.z);

    Eigen::Matrix4d Twc = Eigen::Matrix4d::Identity(); // NOLINT
    Twc.block<3, 3>(0, 0) = q.toRotationMatrix();
    Twc.block<3, 1>(0, 3) = t;

    glPushMatrix();
    glMultMatrixd(Twc.transpose().data());
    pangolin::glDrawAxis(0.5);
    glPopMatrix();
}

void PangolinViewerAdapterImpl::drawGrid()
{
    glColor3f(0.3F, 0.3F, 0.3F);
    glLineWidth(1.0F);

    glBegin(GL_LINES);
    for (int i = -10; i <= 10; ++i)
    {
        glVertex3f(static_cast<float>(i), 0.0F, -10.0F);
        glVertex3f(static_cast<float>(i), 0.0F, 10.0F);

        glVertex3f(-10.0F, 0.0F, static_cast<float>(i));
        glVertex3f(10.0F, 0.0F, static_cast<float>(i));
    }
    glEnd();
}

} // namespace vp::adapter::out