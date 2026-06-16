!
! ifort -o matvec_server -I${HERMES_HOME}/include matvec_rpc.f90 \
!       matvec_server.f90 -L${HERMES_HOME}/lib -lhermes_fortran \
!       -L${ZEROMQ_HOME}/lib -lzmq
!
module matvec_server_mod
  use matvec_rpc, only : matvec_server,                                        &
                         problem
  use, intrinsic :: iso_c_binding, only : c_ptr, c_null_ptr

  implicit none

  private

  type, public, extends(matvec_server) :: server_t
    contains
      procedure, public :: dgemv
  end type

  logical,       public :: done = .false.

contains

  !-----------------------------  INTERFACE -----------------------------------

  function dgemv(self, a_p) result(r)
    use, intrinsic :: iso_c_binding, only : c_double, c_size_t

    class(server_t)                                               :: self
    type(problem),                                    intent(in)  :: a_p
    real(kind = c_double), dimension(:), allocatable              :: r

    integer(c_size_t)                                             :: mdim, vdim

continue

    mdim = a_p%size_a()
    vdim = a_p%size_x()

    if ( mdim < vdim * vdim ) then
      write(*,*) 'Bad problem dimensions:',mdim,vdim
    endif

    allocate(r(vdim))

    r(1) = a_p%get_a(1) * a_p%get_x(1) +                                       &
           a_p%get_a(2) * a_p%get_x(2) +                                       &
           a_p%get_a(3) * a_p%get_x(3)
    r(2) = a_p%get_a(4) * a_p%get_x(1) +                                       &
           a_p%get_a(5) * a_p%get_x(2) +                                       &
           a_p%get_a(6) * a_p%get_x(3)
    r(3) = a_p%get_a(7) * a_p%get_x(1) +                                       &
           a_p%get_a(8) * a_p%get_x(2) +                                       &
           a_p%get_a(9) * a_p%get_x(3)

  end function dgemv

end module matvec_server_mod

program server
  use, intrinsic :: iso_c_binding, only : c_ptr, c_int32_t
  use matvec_server_mod, only : server_t, done
  use zmq,               only : zmq_ctx_new, zmq_ctx_term, ZMQ_REP

  implicit none

  type(c_ptr)                                     :: context
  type(server_t),                  target         :: srv
  integer(kind = c_int32_t)                       :: code

continue

  context = zmq_ctx_new()
  code = srv%open(context, 'tcp://*:49200', ZMQ_REP)
  if (0 > code) then
    write(*,*) 'Error: failed to bind control server'
    stop
  endif
  write(*,*) 'Starting server...'

  do while(.not. done)
    call srv%serve_once()
  enddo

  code = srv%close()
  code = zmq_ctx_term(context)

end program server
