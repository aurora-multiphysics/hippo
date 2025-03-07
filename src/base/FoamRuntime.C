#include "FoamRuntime.h"
#include "fvCFD_moose.h"

#include <MooseError.h>

namespace Hippo
{

namespace
{
cArgs
make_foam_init_args(const std::string & case_dir, MPI_Comm const & comm)
{
  cArgs args("foamRun");
  args.push_arg("-case");
  args.push_arg(case_dir);
  int mpi_world_size{1};
  MPI_Comm_size(comm, &mpi_world_size);
  if (mpi_world_size > 1)
  {
    args.push_arg("-parallel");
  }
  return args;
}

Foam::argList
make_arg_list(cArgs & argv, MPI_Comm const & comm)
{
  int size;
  MPI_Comm_size(comm, &size);
  auto arg_list = Foam::argList(argv.get_argc(), argv.get_argv_ptr(), (void *)&comm);
  if (!arg_list.checkRootCase())
  {
    int rank;
    MPI_Comm_rank(comm, &rank);
    if (!arg_list.checkRootCase())
    {
      mooseWarning("OpenFOAM failed root case check rank: ", rank);
    }
  }
  return arg_list;
}
} // namespace

FoamRuntime::FoamRuntime(const std::string & case_dir, MPI_Comm const & comm)
  : _argv(make_foam_init_args(case_dir, comm)),
    _runtime(Foam::Time::controlDictName, make_arg_list(_argv, comm))
{
}

FoamRuntime::FoamRuntime(const FoamRuntime & rt)
  : _argv(rt._argv),
    _runtime(Foam::Time::controlDictName, make_arg_list(_argv, Foam::PstreamGlobals::MPI_COMM_FOAM))
{
}

}
