/*
  @file

  @author victor

  @copyright (c) 2018 PAL Robotics SL. All Rights Reserved
*/
#include <pal_gazebo_plugins/gazebo_attachment.h>
#include <gazebo/math/gzmath.hh>
#include <sdf/sdf.hh>
#include <ros/ros.h>

namespace gazebo
{
GazeboAttachment::GazeboAttachment()
  : load_ok_(false)
{
  
}

void GazeboAttachment::Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf)
{
  this->model_ = _parent;
  this->world_ = _parent->GetWorld();

  if (_sdf->HasElement("target_model_name"))
  {
    this->target_model_ = this->world_->GetModel(_sdf->Get<std::string>("target_model_name"));
    if (!this->target_model_.get())
    {
    ROS_ERROR_STREAM(
        "Got non-existing target_model_name: " << _sdf->Get<std::string>("target_model_name"));
    return;
    }
  }
  else
  {
    ROS_ERROR_STREAM("Didn't find target_model_name");
    return;
  }
  if (_sdf->HasElement("target_link_name"))
  {
    this->target_link_ =
        this->target_model_->GetLink(_sdf->Get<std::string>("target_link_name"));
    if (!this->target_link_.get())
    {
    ROS_ERROR_STREAM("Got non-existing target_link_name: " << _sdf->Get<std::string>("target_link_name"));
    return;
    }
  }
  else
  {
    ROS_ERROR_STREAM("Didn't find target_link_name");
    return;
  }
  if (_sdf->HasElement("local_link_name"))
  {
    this->local_link_ = this->model_->GetLink(_sdf->Get<std::string>("local_link_name"));
    if (!this->local_link_.get())
    {
      
      ROS_ERROR_STREAM("Got non-existing local_link_name: " << _sdf->Get<std::string>("local_link_name"));
      return;
    }
  }
  else
  {
    ROS_ERROR_STREAM("Didn't find local_link_name");
    return;
  }

  if (_sdf->HasElement("pose"))
  {
    this->pose_ = _sdf->Get<ignition::math::Pose3d>("pose");
  }
  else
  {
    ROS_ERROR_STREAM("Didn't find pose");
    return;
  }
  this->load_ok_ = true;
}



void gazebo::GazeboAttachment::Init()
{
  if (!this->load_ok_)
  {
    ROS_ERROR("Not doing attachment beacuse GazeboAttachment plugin had configuration errors");
    return;
  }
  const auto p = this->target_link_->GetWorldPose();
  ignition::math::Pose3d target_link_pose =
      ignition::math::Pose3d(p.pos.x, p.pos.y, p.pos.z, p.rot.w, p.rot.x, p.rot.y, p.rot.z);

  ignition::math::Pose3d target_pose =
      ignition::math::Pose3d(target_link_pose.Pos() + this->pose_.Pos(),
                             target_link_pose.Rot() * this->pose_.Rot());
  this->model_->SetLinkWorldPose(target_pose, this->local_link_);

  physics::JointPtr joint =
      this->world_->GetPhysicsEngine()->CreateJoint("revolute", this->model_);
  joint->Attach(this->local_link_, this->target_link_);
  joint->Load(this->local_link_, this->target_link_, math::Pose());
  joint->SetModel(this->model_);
  joint->SetHighStop(0, 0);
  joint->SetLowStop(0, 0);
  joint->Init();
}

GZ_REGISTER_MODEL_PLUGIN(GazeboAttachment)
}  // namespace gazebo