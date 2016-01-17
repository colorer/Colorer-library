#include <common/Colorer.h>

Colorer::~Colorer()
{
}

std::unique_ptr<Colorer> Colorer::createColorer()
{
  return std::unique_ptr<Colorer>(new Colorer);
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

}
