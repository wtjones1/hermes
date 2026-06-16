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
    integer(c_size_t)                                             :: n, i, j
    integer(c_size_t)                                             :: idx

continue

    mdim = a_p%size_a()
    vdim = a_p%size_x()

    if ( 0 /= mod(mdim,vdim) ) then
      write(*,*) 'Bad problem dimensions:',mdim,vdim
    endif

    n = mdim / vdim

    allocate(r(n))

    do i = 1, n
      r(i) = 0.0_c_double
      do j = 1, vdim
        idx = (i - 1) * vdim + j
        r(i) = r(i) + a_p%get_a(idx) * a_p%get_x(j)
      end do
    end do

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
