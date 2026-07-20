/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/

#include "register_types.h"

#include "agent_trace.h"
#include "artifact/agent_artifact_graph.h"
#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "validation/agent_contract_validator.h"

static AgentTrace *agent_trace = nullptr;
static AgentArtifactGraph *agent_artifact_graph = nullptr;
static AgentContractValidator *agent_contract_validator = nullptr;

void initialize_agent_debugger_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_CLASS(AgentTrace);
	GDREGISTER_CLASS(AgentArtifactGraph);
	GDREGISTER_CLASS(AgentContractValidator);
	agent_trace = memnew(AgentTrace);
	agent_artifact_graph = memnew(AgentArtifactGraph);
	agent_contract_validator = memnew(AgentContractValidator);
	Engine::get_singleton()->add_singleton(Engine::Singleton("AgentTrace", agent_trace));
	Engine::get_singleton()->add_singleton(Engine::Singleton("AgentArtifactGraph", agent_artifact_graph));
	Engine::get_singleton()->add_singleton(Engine::Singleton("AgentContractValidator", agent_contract_validator));
}

void uninitialize_agent_debugger_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	Engine::get_singleton()->remove_singleton("AgentContractValidator");
	Engine::get_singleton()->remove_singleton("AgentArtifactGraph");
	Engine::get_singleton()->remove_singleton("AgentTrace");
	memdelete(agent_contract_validator);
	memdelete(agent_artifact_graph);
	memdelete(agent_trace);
	agent_contract_validator = nullptr;
	agent_artifact_graph = nullptr;
	agent_trace = nullptr;
}
