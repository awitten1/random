#pragma once

#include "csi.grpc.pb.h"


namespace csi {

class IdentityImpl final : public csi::v1::Identity::Service {
  grpc::Status GetPluginInfo(grpc::ServerContext* context, const csi::v1::GetPluginInfoRequest* request, csi::v1::GetPluginInfoResponse* response);
  grpc::Status GetPluginCapabilities(grpc::ServerContext* context, const csi::v1::GetPluginCapabilitiesRequest* request, csi::v1::GetPluginCapabilitiesResponse* response);
  grpc::Status Probe(grpc::ServerContext* context, const csi::v1::ProbeRequest* request, csi::v1::ProbeResponse* response);
};


class NodeImpl final : public csi::v1::Node::Service {
  grpc::Status NodeStageVolume(grpc::ServerContext* context, const csi::v1::NodeStageVolumeRequest* request, csi::v1::NodeStageVolumeResponse* response);
  grpc::Status NodeUnstageVolume(grpc::ServerContext* context, const csi::v1::NodeUnstageVolumeRequest* request, csi::v1::NodeUnstageVolumeResponse* response);
  grpc::Status NodePublishVolume(grpc::ServerContext* context, const csi::v1::NodePublishVolumeRequest* request, csi::v1::NodePublishVolumeResponse* response);
  grpc::Status NodeUnpublishVolume(grpc::ServerContext* context, const csi::v1::NodeUnpublishVolumeRequest* request, csi::v1::NodeUnpublishVolumeResponse* response);
  grpc::Status NodeGetVolumeStats(grpc::ServerContext* context, const csi::v1::NodeGetVolumeStatsRequest* request, csi::v1::NodeGetVolumeStatsResponse* response);
  grpc::Status NodeExpandVolume(grpc::ServerContext* context, const csi::v1::NodeExpandVolumeRequest* request, csi::v1::NodeExpandVolumeResponse* response);
  grpc::Status NodeGetCapabilities(grpc::ServerContext* context, const csi::v1::NodeGetCapabilitiesRequest* request, csi::v1::NodeGetCapabilitiesResponse* response);
  grpc::Status NodeGetInfo(grpc::ServerContext* context, const csi::v1::NodeGetInfoRequest* request, csi::v1::NodeGetInfoResponse* response);
};

}
