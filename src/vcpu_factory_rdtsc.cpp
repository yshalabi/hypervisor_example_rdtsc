//
// Bareflank Hypervisor
// Copyright (C) 2015 Assured Information Security, Inc.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#include <bfvmm/hve/arch/intel_x64/vcpu.h>
#include <bfvmm/vcpu/vcpu_factory.h>
using namespace ::intel_x64::vmcs;

namespace example {
namespace  intel_x64 {

//class vmcs_rdtsc : public vmcs_intel_x64
class vcpu : public bfvmm::intel_x64::vcpu
{
public:

    vcpu(vcpuid::type id) : bfvmm::intel_x64::vcpu{id}
    {
        exit_handler()->add_handler(
            exit_reason::basic_exit_reason::rdtscp,
            ::handler_delegate_t::create<vcpu, &vcpu::handle_rdtscp_exit>(this)
        );
        exit_handler()->add_handler(
            exit_reason::basic_exit_reason::rdtsc,
            ::handler_delegate_t::create<vcpu, &vcpu::handle_rdtsc_exit>(this)
        );
        primary_processor_based_vm_execution_controls::rdtsc_exiting::enable();
    }

    bool handle_rdtsc_exit(gsl::not_null<bfvmm::intel_x64::vmcs *> vmcs)
    {
        auto * m_state_save = vmcs->save_state();
        auto ret = x64::read_tsc::get();
        m_state_save->rax = set_bits(m_state_save->rax, 0x00000000FFFFFFFF, ret >> 0);
        m_state_save->rdx = set_bits(m_state_save->rdx, 0x00000000FFFFFFFF, ret >> 32);
        return advance(vmcs);
    }

    bool handle_rdtscp_exit(gsl::not_null<bfvmm::intel_x64::vmcs *> vmcs)
    {
        auto * m_state_save = vmcs->save_state();
        auto ret = x64::read_tscp::get();
        m_state_save->rax = set_bits(m_state_save->rax, 0x00000000FFFFFFFF, ret >> 0);
        m_state_save->rdx = set_bits(m_state_save->rdx, 0x00000000FFFFFFFF, ret >> 32);
        m_state_save->rcx = set_bits(m_state_save->rcx, 0x00000000FFFFFFFF, x64::msrs::ia32_tsc_aux::get());
        return advance(vmcs);
    }

};
}
}
namespace bfvmm
{

WEAK_SYM std::unique_ptr<vcpu>
vcpu_factory::make(vcpuid::type vcpuid, bfobject *obj)
{
    bfignored(obj);
    return std::make_unique<example::intel_x64::vcpu>(vcpuid);
}

}

