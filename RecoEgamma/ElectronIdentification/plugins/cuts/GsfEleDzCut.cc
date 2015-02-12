#include "PhysicsTools/SelectorUtils/interface/CutApplicatorWithEventContentBase.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"

class GsfEleDzCut : public CutApplicatorWithEventContentBase {
public:
  GsfEleDzCut(const edm::ParameterSet& c);
  
  result_type operator()(const reco::GsfElectronPtr&) const override final;

  void setConsumes(edm::ConsumesCollector&) override final;
  void getEventContent(const edm::EventBase&) override final;

  CandidateType candidateType() const override final { 
    return ELECTRON; 
  }

private:  
  const double _dzCutValueEB,_dzCutValueEE;
  const double _barrelCutOff=1.479;
  edm::Handle<reco::VertexCollection> _vtxs;
};

DEFINE_EDM_PLUGIN(CutApplicatorFactory,
		  GsfEleDzCut,
		  "GsfEleDzCut");

GsfEleDzCut::GsfEleDzCut(const edm::ParameterSet& c) :
  CutApplicatorWithEventContentBase(c),
  _dzCutValueEB(c.getParameter<double>("dzCutValueEB")),
  _dzCutValueEE(c.getParameter<double>("dzCutValueEE")) {
  edm::InputTag vertextag = c.getParameter<edm::InputTag>("vertexSrc");
  edm::InputTag vertextagMA = c.getParameter<edm::InputTag>("vertexSrcMiniAOD");
  contentTags_.emplace("vertices",vertextag);
  contentTags_.emplace("verticesMA",vertextagMA);
}

void GsfEleDzCut::setConsumes(edm::ConsumesCollector& cc) {
  auto vtcs = cc.mayConsume<reco::VertexCollection>(contentTags_["vertices"]);
  auto vtcsMA = cc.mayConsume<reco::VertexCollection>(contentTags_["verticesMA"]);
  contentTokens_.emplace("vertices",vtcs);
  contentTokens_.emplace("verticesMA",vtcsMA);
}

void GsfEleDzCut::getEventContent(const edm::EventBase& ev) {    
 // First try AOD, then go to miniAOD. Use the same Handle since collection class is the same.
  ev.getByLabel(contentTags_["vertices"],_vtxs);
  if (!_vtxs.isValid())
    ev.getByLabel(contentTags_["verticesMA"],_vtxs);
}

CutApplicatorBase::result_type 
GsfEleDzCut::
operator()(const reco::GsfElectronPtr& cand) const{  
  const float dzCutValue = 
    ( std::abs(cand->superCluster()->position().eta()) < _barrelCutOff ? 
      _dzCutValueEB : _dzCutValueEE );
  
  const reco::VertexCollection& vtxs = *_vtxs;
  const double dz = ( vtxs.size() ? 
		      cand->gsfTrack()->dz(vtxs[0].position()) : 
		      cand->gsfTrack()->dz() );
  return std::abs(dz) < dzCutValue;
}
