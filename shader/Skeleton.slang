module Skeleton;

import Common;

public cbuffer joints
{
    SkeletalJointMat joints;
}
public cbuffer prev_joints
{
	SkeletalJointMat prev_joints;
}

public float4x4 CalcSkeletonMatrix(float4 blend_weight, uint4 blend_index)
{	
	public float4x4 skin_mat = blend_weight.x * joints.joint_mat[blend_index.x];
	skin_mat += blend_weight.y * joints.joint_mat[blend_index.y];
	skin_mat += blend_weight.z * joints.joint_mat[blend_index.z];
	skin_mat += blend_weight.w * joints.joint_mat[blend_index.w];
	
	return skin_mat;
}

public float4x4 CalcSkeletonMatrixEx(float4 blend_weight, uint4 blend_index, float4 blend_weight1, uint4 blend_index1)
{	
	public float4x4 skin_mat = blend_weight.x * joints.joint_mat[blend_index.x];
	skin_mat += blend_weight.y * joints.joint_mat[blend_index.y];
	skin_mat += blend_weight.z * joints.joint_mat[blend_index.z];
	skin_mat += blend_weight.w * joints.joint_mat[blend_index.w];
	
	skin_mat += blend_weight1.x * joints.joint_mat[blend_index1.x];
	skin_mat += blend_weight1.y * joints.joint_mat[blend_index1.y];
	skin_mat += blend_weight1.z * joints.joint_mat[blend_index1.z];
	skin_mat += blend_weight1.w * joints.joint_mat[blend_index1.w];
	
	return skin_mat;
}

public float4x4 CalcPrevSkeletonMatrix(float4 blend_weight, uint4 blend_index)
{	
	public float4x4 skin_mat = blend_weight.x * prev_joints.joint_mat[blend_index.x];
	skin_mat += blend_weight.y * prev_joints.joint_mat[blend_index.y];
	skin_mat += blend_weight.z * prev_joints.joint_mat[blend_index.z];
	skin_mat += blend_weight.w * prev_joints.joint_mat[blend_index.w];
	
	return skin_mat;
}

public float4x4 CalcPrevSkeletonMatrixEx(float4 blend_weight, uint4 blend_index, float4 blend_weight1, uint4 blend_index1)
{	
	public float4x4 skin_mat = blend_weight.x * prev_joints.joint_mat[blend_index.x];
	skin_mat += blend_weight.y * prev_joints.joint_mat[blend_index.y];
	skin_mat += blend_weight.z * prev_joints.joint_mat[blend_index.z];
	skin_mat += blend_weight.w * prev_joints.joint_mat[blend_index.w];
	
	skin_mat += blend_weight1.x * prev_joints.joint_mat[blend_index1.x];
	skin_mat += blend_weight1.y * prev_joints.joint_mat[blend_index1.y];
	skin_mat += blend_weight1.z * prev_joints.joint_mat[blend_index1.z];
	skin_mat += blend_weight1.w * prev_joints.joint_mat[blend_index1.w];
	
	return skin_mat;
}
