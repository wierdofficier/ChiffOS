#include <types.h>
#include <v86mon.h>
#include <kutils.h>
#include <proc.h>
static volatile current_t Current;
int status;
#define BIT(n) (1<<(n))
static volatile current_t Current;
int status;
bool vm86sensitiveOpcodehandler(struct regs *ctx)
{
    u8*  ip      = FP_TO_LINEAR(ctx->cs, ctx->eip);
    u16* ivt     = 0;
    u16* stack   = (u16*)FP_TO_LINEAR(ctx->ss, ctx->useresp);
    u32* stack32 = (u32*)stack;
    bool isOperand32 = false;

    while (true)
    {
        switch (ip[0])
        {
          case 0x66: 
        
            isOperand32 = true;
            ip++;
            ctx->eip++;
            break;

        case 0x67: // A32
          
            ip++;
            ctx->eip++;
            break;

        case 0x9C: 
        
            if (isOperand32)
            {
                ctx->useresp = ((ctx->useresp & 0xFFFF) - 4) & 0xFFFF;
                stack32--;
                stack32[0] = ctx->eflags & VALID_FLAGS;

                if (Current.v86_if)
                {
                    stack32[0] |= EFLAG_IF;
                }
                else
                {
                    stack32[0] &= ~EFLAG_IF;
                }
            }
            else
            {
                ctx->useresp = ((ctx->useresp & 0xFFFF) - 2) & 0xFFFF;
                stack--;
                stack[0] = (u16) ctx->eflags;

                if (Current.v86_if)
                {
                    stack[0] |= EFLAG_IF;
                }
                else
                {
                    stack[0] &= ~EFLAG_IF;
                }
            }
            ctx->eip++;
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            return true;

        case 0x9D: // POPF
         

            if (isOperand32)
            {
                ctx->eflags = EFLAG_IF | EFLAG_VM | (stack32[0] & VALID_FLAGS);
                Current.v86_if = (stack32[0] & EFLAG_IF) != 0;
                ctx->useresp = ((ctx->useresp & 0xFFFF) + 4) & 0xFFFF;
            }
            else
            {
                ctx->eflags = EFLAG_IF | EFLAG_VM | stack[0];
                Current.v86_if = (stack[0] & EFLAG_IF) != 0;
                ctx->useresp = ((ctx->useresp & 0xFFFF) + 2) & 0xFFFF;
            }
            ctx->eip++;
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            return true;
                    case 0xEF: 
            if (!isOperand32)
            {
           
                outw(ctx->edx, ctx->eax);
            }
            else
            {
             
                outl(ctx->edx, ctx->eax);
            }
            ctx->eip++;
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            return true;

case 0xEE: // OUT DX, AL
         
            outb(ctx->edx, ctx->eax);
            ctx->eip++;
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            return true;

        case 0xED: // IN AX,DX and IN EAX,DX
            if (!isOperand32)
            {
              
                ctx->eax = (ctx->eax & 0xFFFF0000) + inw(ctx->edx);
            }
            else
            {
             
                ctx->eax = inl(ctx->edx);
            }
            ctx->eip++;
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            return true;

        case 0xEC: // IN AL,DX
        
            ctx->eax = (ctx->eax & 0xFF00) + inb(ctx->edx);
            ctx->eip++;
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            return true;


        case 0xCD: 
        
            switch (ip[1])
            {
            case 0x30:
           
                return true;

            case 0x20:
            case 0x21:
                return false;

            default:
                stack -= 3;
                ctx->useresp = ((ctx->useresp & 0xFFFF) - 6) & 0xFFFF;
                stack[2] = (u16) (ctx->eip + 2);
                stack[1] = ctx->cs;
                stack[0] = (u16) ctx->eflags;

                if (Current.v86_if)
                {
                    stack[0] |= EFLAG_IF;
                }
                else
                {
                    stack[0] &= ~EFLAG_IF;
                }
                ctx->eip = ivt[2 * ip[1]    ];
                ctx->cs  = ivt[2 * ip[1] + 1];
                ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
         
                return true;
            }
            break;

        case 0xCF: // IRET
    
            ctx->eip    = stack[2];
            ctx->cs     = stack[1];
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            ctx->eflags = EFLAG_IF | EFLAG_VM | stack[0];
            ctx->useresp    = ((ctx->useresp & 0xFFFF) + 6) & 0xFFFF;

            Current.v86_if = (stack[0] & EFLAG_IF) != 0;

            return true;

        case 0xFA: // CLI
  
            Current.v86_if = false;
            ctx->eip++;
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            return true;

        case 0xFB: // STI
     
            Current.v86_if = true;
            ctx->eip++;
            ip = FP_TO_LINEAR(ctx->cs, ctx->eip);
            return true;

       case 0xF4: // HLT
			
            exit();
            return true; 

        default:
       
            return false;
        }
    }
    return false;
}



