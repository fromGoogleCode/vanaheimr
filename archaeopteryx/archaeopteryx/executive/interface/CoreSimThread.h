/*! \file   CoreSimThread.h
	\date   Saturday Feburary 23, 2011
	\author Gregory and Sudnya Diamos
		<gregory.diamos@gatech.edu, mailsudnya@gmail.com>
	\brief  The header file for the Core simulator of the thread class.
*/

#pragma once

/*! \brief A namespace for program execution */
namespace executive
{
class CoreSimThread
{
    public:
        typedef ir::Binary::PC PC;
    public:
        __device__ CoreSimThread(CoreSimBlock* parentBlock, unsigned threadId);
        __device__ PC executeInstruction(ir::Instruction*, PC);
    private:
        CoreSimBlock* m_parentBlock;
        unsigned m_tId;
};

}
