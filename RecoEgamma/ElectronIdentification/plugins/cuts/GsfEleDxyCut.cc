#include "PhysicsTools/SelectorUtils/interface/CutApplicatorWithEventContentBase.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"

class GsfEleDxyCut : public CutApplicatorWithEventContentBase {
public:
  GsfEleDxyCut(const edm::ParameterSet& c);
  
  result_type operator()(const reco::GsfElectronPtr&) const override final;

  void setConsumes(edm::ConsumesCollector&) override final;
  void getEventContent(const edm::EventBase&) override final;

  CandidateType candidateType() const override final { 
    return ELECTRON; 
  }

private:  
  const double _dxyCutValueEB, _dxyCutValueEE;
  const double _barrelCutOff = 1.479;
  edm::Handle<reco::VertexCollection> _vtxs;
};

DEFINE_EDM_PLUGIN(CutApplicatorFactory,
		  GsfEleDxyCut,
		  "GsfEleDxyCut");

GsfEleDxyCut::GsfEleDxyCut(const edm::ParameterSet& c) :
  CutApplicatorWithEventContentBase(c),
  _dxyCutValueEB(c.getParameter<double>("dxyCutValueEB")),
  _dxyCutValueEE(c.getParameter<double>("dxyCutValueEE")) {
  edm::InputTag vertextag = c.getParameter<edm::InputTag>("vertexSrc");
  edm::InputTag vertextagMA = c.getParameter<edm::InputTag>("vertexSrcMiniAOD"); 
  contentTags_.emplace("vertices",vertextag);
  contentTags_.emplace("verticesMA",vertextagMA);
}

void GsfEleDxyCut::setConsumes(edm::ConsumesCollector& cc) {
  auto vtcs = cc.mayConsume<reco::VertexCollection>(contentTags_["vertices"]);
  auto vtcsMA = cc.mayConsume<reco::VertexCollection>(contentTags_["verticesMA"]);
  contentTokens_.emplace("vertices",vtcs);
  contentTokens_.emplace("verticesMA",vtcsMA);
}

void GsfEleDxyCut::getEventContent(const edm::EventBase& ev) {    
  // First try AOD, then go to miniAOD. Use the same Handle since collection class is the same.
  ev.getByLabel(contentTags_["vertices"],_vtxs);
  if (!_vtxs.isValid())
    ev.getByLabel(contentTags_["verticesMA"],_vtxs);
}

CutApplicatorBase::result_type 
GsfEleDxyCut::
operator()(const reco::GsfElectronPtr& cand) const{  
  const float dxyCutValue = 
    ( std::abs(cand->superCluster()->position().eta()) < _barrelCutOff ? 
      _dxyCutValueEB : _dxyCutValueEE );

  const reco::VertexCollection& vtxs = *_vtxs;
  const double dxy = ( vtxs.size() ? 
		       cand->gsfTrack()->dxy(vtxs[0].position()) : 
		       cand->gsfTrack()->dxy() );
  return std::abs(dxy) < dxyCutValue;
}
