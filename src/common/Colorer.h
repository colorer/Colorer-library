#ifndef _COLORER_COLORER_H_
#define _COLORER_COLORER_H_

#include <memory>
#include <g3log/logworker.hpp>

class Colorer
{
  Colorer() = default;

public:
  ~Colorer();
  static std::unique_ptr<Colorer> createColorer();
  void initColorer(g3::LogWorker* _log_worker = nullptr);

private:
  std::unique_ptr<g3::LogWorker> log_worker;
};

/* empty logger*/
struct VoidReceiver {
  explicit VoidReceiver() {}

  void receiveMsg(g3::LogMessageMover msg)
  {
    /*ignored*/
  }
};

#endif //_COLORER_COLORER_H_
