#include<iostream>
#include<fstream>
#include<string>
#include<deque>

using namespace std;

int reg[16], stale[16];
int PC;
char IR[4];
int instructions, arithmetic, logical, data, control, halt;
int cycles;
bool completed, control_stalled;
int stalls, data_stalls, control_stalls;

//coverts from hexadecimal to decimal
int hex_to_dec(char hexval)
{
    if (hexval <= '9')
        return (hexval - '0');
    return (hexval - 'a' + 10);
}

//converts from decimal to hexadecimal
string dec_to_hex(int decval)
{
    int hex[2];
    hex[0] = decval & 15;
    hex[1] = (decval & 240)>>4;
    string hexval;
    for(int i = 1; i >= 0; i--)
    {
        if(hex[i] <= 9)
            hexval.push_back(hex[i] + '0');
        else
            hexval.push_back(hex[i] - 10 + 'a');
    }
    return hexval;
}

//checks if the given register is stale or not
int check_reg_dependency(int reg_num)
{
    return stale[reg_num];
}

class Instruction
{
public:
    int dest, source1, source2, jump_offset, mem_offset;
    int is_val1, is_val2;
    int instr, ALUOutput, LMD;
    int stage;
    bool data_stall, control_stall;
    Instruction()
    {
        stage = 0;
        data_stall = control_stall = false;
        is_val1 = is_val2 = 1;
    }
    void instruction_fetch();
    void instruction_decode();
    void execute();
    void mem_access();
    void write_back();
    void recheck_dependency(int reg_num);
};

//dequeue containing the instructions in the pipeline
//being processed currently in the reverse order
deque<Instruction> pipeline;


//initializes all the global variables
void initialize()
{
    ifstream RF;
    RF.open("RF.txt");
    string temp;
    for(int i = 0; i < 16; i++)
    {
        RF>>temp;
        reg[i] = hex_to_dec(temp[0]) * 16 + hex_to_dec(temp[1]);
        stale[i] = 0;
    }
    RF.close();
    PC = 0;
    instructions = 0;
    arithmetic = logical = data = control = halt = 0;
    cycles = 0;
    completed = control_stalled = false;
    stalls = data_stalls = control_stalls = 0;
    ifstream DCache;
    ofstream ODCache;
    //copy contents of DCache to ODCache
    DCache.open("DCache.txt");
    ODCache.open("ODCache.txt");
    while(DCache)
    {
        DCache>>temp;
        ODCache<<temp<<endl;
    }
    ODCache.close();
    DCache.close();
    pipeline.clear();
}

//instruction fetch stage of pipeline
void Instruction::instruction_fetch() //stage 0
{
    cout<<"Instruction Fetch stage"<<endl;
    cout<<"Fetched instruction with PC "<<PC<<endl;
    ifstream ICache;
    ICache.open("ICache.txt");
    ICache.seekg(PC*3, ICache.beg);
    ICache>>IR;
    ICache>>&IR[2];
    PC += 2;
    stage++;
}

//instruction decode stage of pipeline
void Instruction::instruction_decode() //stage 1
{
    cout<<"Instruction Decode stage"<<endl;
    int temp;
    //convert the instruction from hexadecimal to integer form
    instr = hex_to_dec(IR[0]);
    switch (instr)
    {
    //all instructions with two input registers
    case 0: // ADD
    case 1: // SUB
    case 2: // MUL
    case 4: // AND
    case 5: // OR
    case 7: // XOR
        temp = hex_to_dec(IR[2]);
        if (check_reg_dependency(temp))
        {
            source1 = temp;
            is_val1 = 0;
            data_stall = true;
        }
        else
        {
            source1 = reg[temp];
            is_val1 = 1;
        }
        temp = hex_to_dec(IR[3]);
        if (check_reg_dependency(temp))
        {
            source2 = temp;
            is_val2 = 0;
            data_stall = true;
        }
        else
        {
            source2 = reg[temp];
            is_val2 = 1;
        }
        dest = hex_to_dec(IR[1]);
        stale[dest]++;
        if(instr < 4)
            arithmetic++;
        else
            logical++;
        break;

    //remaining instructions
    case 3: // INC
        temp = hex_to_dec(IR[1]);
        if (check_reg_dependency(temp))
        {
            source1 = temp;
            is_val1 = 0;
            data_stall = true;
        }
        else
        {
            source1 = reg[temp];
            is_val1 = 1;
        }
        dest = temp;
        stale[dest]++;
        arithmetic++;
        break;
    case 6: // NOT
        temp = hex_to_dec(IR[2]);
        if (check_reg_dependency(temp))
        {
            source1 = temp;
            is_val1 = 0;
            data_stall = true;
        }
        else
        {
            source1 = reg[temp];
            is_val1 = 1;
        }
        dest = hex_to_dec(IR[1]);
        stale[dest]++;
        logical++;
        break;
    case 8: // LOAD
        mem_offset = hex_to_dec(IR[3]);
        temp = hex_to_dec(IR[2]);
        if (check_reg_dependency(temp))
        {
            source1 = temp;
            is_val1 = 0;
            data_stall = true;
        }
        else
        {
            source1 = reg[temp];
            is_val1 = 1;
        }
        dest = hex_to_dec(IR[1]);
        stale[dest]++;
        data++;
        break;
    case 9: // STORE
        mem_offset = hex_to_dec(IR[3]);
        temp = hex_to_dec(IR[1]);
        if (check_reg_dependency(temp))
        {
            source1 = temp;
            is_val1 = 0;
            data_stall = true;
        }
        else
        {
            source1 = reg[temp];
            is_val1 = 1;
        }
        temp = hex_to_dec(IR[2]);
        if (check_reg_dependency(temp)) // CAUTION: source2 contains the base address for memory address calculation in STOREs
        {
            source2 = temp;
            is_val2 = 0;
            data_stall = true;
        }
        else
        {
            source2 = reg[temp];
            is_val2 = 1;
        }
        data++;
        break;
    case 10: // JMP
        //convert jump_offset from 2's complement to integer form
        jump_offset = hex_to_dec(IR[1]) * 16 + hex_to_dec(IR[2]);
        if(jump_offset > 127)
            jump_offset = jump_offset - 256;

        control++;
        control_stall = true;
        control_stalled = true;
        break;
    case 11:                            // BEQZ
        temp = hex_to_dec(IR[1]); // check dependency
        if (check_reg_dependency(temp))
        {
            source1 = temp;
            is_val1 = 0;
            data_stall = true;
        }
        else
        {
            source1 = reg[temp];
            is_val1 = 1;
        }

        //convert jump_offset from 2's complement to interger form
        jump_offset = hex_to_dec(IR[2]) * 16 + hex_to_dec(IR[3]);
        if(jump_offset > 127)
            jump_offset = jump_offset - 256;
        
        control++;
        control_stall = true;
        control_stalled = true;
        break;
    case 15: // HLT
        halt = 1;
        break;
    }
    stage++;
}

//execution - effective address calculation stage of pipeline
void Instruction::execute() //stage 2
{
    cout<<"Execute stage"<<endl;
    switch (instr)
    {
    case(0):    //ADD
        ALUOutput = source1 + source2;
        break;
    
    case(1):    //SUB
        ALUOutput = source1 - source2;
        break;

    case(2):    //MUL
        ALUOutput = source1 * source2;
        break;
    
    case(3):    //INC
        ALUOutput = source1 + 1;
        break;
    
    case(4):    //AND
        ALUOutput = source1 & source2;
        break;
    
    case(5):    //OR
        ALUOutput = source1 | source2;
        break;
    
    case(6):    //NOT
        ALUOutput = ~source1;
        break;
    
    case(7):    //XOR
        ALUOutput = source1 ^ source2;
        break;
    
    case(8):    //LOAD
        ALUOutput = source1 + mem_offset;
        break;
    
    case(9):    //STORE
        ALUOutput = source2 + mem_offset;
        break;
    
    case(10):   //JMP
        PC += (jump_offset << 1);
        control_stall = 0;
        control_stalled = false;
        break;

    case(11):   //BEQZ
        if(source1 == 0)
            PC += (jump_offset << 1);
        control_stall = 0;
        control_stalled = false;
        break;
    }

    stage++;    //move instruction to next stage
}

//memory access stage of pipeline
void Instruction::mem_access() //stage 3
{
    cout<<"Memory Access stage"<<endl;

    fstream DCache;
    DCache.open("ODCache.txt");
    char x[2];
    if(instr == 8)          //load instruction memory access
    {
        DCache.seekg(ALUOutput*3, DCache.beg);
        DCache>>x;
        LMD = hex_to_dec(x[0])*16 + hex_to_dec(x[1]);
    }
    else if(instr == 9)     //store instruction memory access
    {
        DCache.seekg(ALUOutput*3, DCache.beg);
        string temp = dec_to_hex(source1);
        DCache<<temp<<endl;
    }
    stage++;
}

//write back stage of pipeline
void Instruction::write_back() //stage 4
{
    cout<<"Writeback stage"<<endl;
    pipeline.pop_front();
    if(instr < 8)           //register write back
        reg[dest] = ALUOutput;
    else if(instr == 8)     //LMD write back
        reg[dest] = LMD;
    else if(instr == 15)    //Halt
    {
        completed = true;
        return;
    }
    else
        return;
    stale[dest]--;

    if(stale[dest] == 0)
        //Update the is_val for all instructions in the queue
        for(int i = 0; i < pipeline.size(); i++)
            pipeline[i].recheck_dependency(dest);

}

//checks if data stalls have been cleared
void Instruction::recheck_dependency(int reg_num)
{
    if(is_val1 == 0 && source1 == reg_num)
    {
        source1 = reg[reg_num];
        is_val1 = 1;
    }
    if(is_val2 == 0 && source2 == reg_num)
    {
        source2 = reg[reg_num];
        is_val2 = 1;
    }
    if(is_val1 == 1 && is_val2 == 1)
    {
        data_stall = false;
    }
}

//prints statistcs
void print_stats()
{
    instructions = arithmetic + logical + data + control + halt;
    float cpi = float(cycles)/instructions;

    ofstream output;
    output.open("Output.txt");
    
    output<<"Total number of instructions executed: "<<instructions<<endl;
    output<<"Number of instructions in each class"<<endl;
    output<<"Arithmetic instructions              : "<<arithmetic<<endl;
    output<<"Logical instructions                 : "<<logical<<endl;
    output<<"Data instructions                    : "<<data<<endl;
    output<<"Control instructions                 : "<<control<<endl;
    output<<"Halt instructions                    : "<<halt<<endl;
    output<<"Cycles Per Instruction               : "<<cpi<<endl;
    output<<"Total number of stalls               : "<<stalls<<endl;
    output<<"Data stalls (RAW)                    : "<<data_stalls<<endl;
    output<<"Control stalls                       : "<<control_stalls<<endl;

    output.close();
}

int main()
{
    initialize();

    //main loop of the processor
    while(1)
    {
        //exit condition
        if(completed)
            break;
        
        cycles++;
        cout<<"Cycle "<<cycles<<":"<<endl;

        bool curr_data_stall = false, curr_control_stall = false;

        //check for stalls in the current cycle
        for(int i = 0; i < pipeline.size(); i++)
            if(pipeline[i].data_stall)
                curr_data_stall = true;
            else if(pipeline[i].control_stall)
                curr_control_stall = true;

        
        //if there are no stalls, pushes the next instruction into the pipeline
        if(!halt && !curr_data_stall && !curr_control_stall)
        {
            Instruction* temp = new Instruction();
            pipeline.push_back(*temp);
        }
        


        for(auto itr = pipeline.begin(); itr != pipeline.end(); itr++)
        {

            if(itr->stage == 2 && curr_data_stall) //We have discovered a data stall in the previous cycle and hence cannot perform the first 3 stages of the pipeline
            {
                data_stalls++;
                stalls++;
                cout<<"Data stall"<<endl;
                break;
            }

            if(itr->stage == 2 && itr->control_stall) //We have discovered a control stall in the previous cycle and need to clear it by performing execute, but cannot decode or fetch
            {
                itr->execute();
                control_stalls++;
                stalls++;
                cout<<"Control stall"<<endl;
                break;
            }

            if(itr->stage == 0 && control_stalled) //We have just discovered a control instruction in decode stage and cannot fetch a new instruction
            {
                control_stalls++;
                stalls++;
                pipeline.pop_back(); //Remove the newly inserted instruction from the pipeline
                cout<<"Control stall"<<endl;
                break;
            }

            //if the current cycle has no stalls
            switch(itr->stage)
            {
                case(0):
                    if(!halt)
                        itr->instruction_fetch();
                    break;
                
                case(1):
                    itr->instruction_decode();
                    break;
                
                case(2):
                    itr->execute();
                    break;
                
                case(3):
                    itr->mem_access();
                    break;
                
                case(4):
                    itr->write_back();
                    break;
                
            }
        }
    }

    print_stats();

    return 0;
}
