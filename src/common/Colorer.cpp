#include <xercesc/dom/DOM.hpp>
#include <common/Colorer.h>

Colorer::Colorer(g3::LogWorker* log_worker)
{
  initColorer(log_worker);
}

Colorer::~Colorer()
{
  // закрываем xerces, подчищая память
  xercesc::XMLPlatformUtils::Terminate();
}

std::unique_ptr<Colorer> Colorer::createColorer(g3::LogWorker* log_worker)
{
  return std::unique_ptr<Colorer>(new Colorer(log_worker));
}

void Colorer::initColorer(g3::LogWorker* _log_worker)
{
  if (!_log_worker) {
    // защита от использования без иницализации логирования
    log_worker = g3::LogWorker::createLogWorker();
    auto handle = log_worker->addSink(std2::make_unique<VoidReceiver>(), &VoidReceiver::receiveMsg);
    g3::only_change_at_initialization::disableAll();
    g3::initializeLogging(log_worker.get());
  }

  // инициализация xerces, иначе будут ошибки работы со строками
  xercesc::XMLPlatformUtils::Initialize();
}
