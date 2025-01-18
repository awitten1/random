
#include "server.h"

using namespace csi::v1;

namespace csi {

grpc::Status IdentityImpl::GetPluginInfo(grpc::ServerContext* context, const GetPluginInfoRequest* request, GetPluginInfoResponse* response) {

}

grpc::Status IdentityImpl::GetPluginCapabilities(grpc::ServerContext* context, const GetPluginCapabilitiesRequest* request, GetPluginCapabilitiesResponse* response) {

}

grpc::Status IdentityImpl::Probe(grpc::ServerContext* context, const ProbeRequest* request, ProbeResponse* response) {

}


grpc::Status NodeImpl::NodeStageVolume(grpc::ServerContext* context, const NodeStageVolumeRequest* request, NodeStageVolumeResponse* response) {

}

grpc::Status NodeImpl::NodeUnstageVolume(grpc::ServerContext* context, const NodeUnstageVolumeRequest* request, NodeUnstageVolumeResponse* response) {

}

grpc::Status NodeImpl::NodePublishVolume(grpc::ServerContext* context, const NodePublishVolumeRequest* request, NodePublishVolumeResponse* response) {

}

grpc::Status NodeImpl::NodeUnpublishVolume(grpc::ServerContext* context, const NodeUnpublishVolumeRequest* request, NodeUnpublishVolumeResponse* response) {

}

grpc::Status NodeImpl::NodeGetVolumeStats(grpc::ServerContext* context, const NodeGetVolumeStatsRequest* request, NodeGetVolumeStatsResponse* response) {

}

grpc::Status NodeImpl::NodeExpandVolume(grpc::ServerContext* context, const NodeExpandVolumeRequest* request, NodeExpandVolumeResponse* response) {

}

grpc::Status NodeImpl::NodeGetCapabilities(grpc::ServerContext* context, const NodeGetCapabilitiesRequest* request, NodeGetCapabilitiesResponse* response) {

}

grpc::Status NodeImpl::NodeGetInfo(grpc::ServerContext* context, const NodeGetInfoRequest* request, NodeGetInfoResponse* response) {

}

}
