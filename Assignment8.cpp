#include <bits/stdc++.h>
using namespace std;

int Prog_Counter=0, Instruction_Cache[256], Data_Cache[256], Register_File[16], if_reg_free[16];

int stall_count = 0, data_stall_count = 0, control_stall_count = 0, instr_count = 0, cycles_count = 0, arithmetic_count = 0, logical_count = 0, control_count = 0, data_count = 0, halts_count = 0;         // Statistics

int halt = 0, fetch_allowed = 1;

int PC=0;//program counter

int_least16_t convert_to_32 (int val)
{
    if (val & (1 << 7)) val = val - (1 << 8);
	//if val is negative converting it to 32 - bit 2's complement form
	return val;
}

int convert_to_8 (int val)
{
    if (val < 0) val = val + (1 << 8);
	//if val is negative converting it to 8 - bit 2's complement form
	return val;
}

class Instr
{
public:
    int IR, instr_pc;
    int pipeline_stage=0;
    int opcode, R1, R2, R3;
    int src_reg1, src_reg2, Immediate_val, ALU_Output, LMD;
    int stall = 0, eqz;
    int ret_stall()
    {
      return stall;
    }
    void fetch_cycle()
    {
        instr_pc = PC;
        PC = PC+2;
        IR = (Instruction_Cache[instr_pc]<<8) + (Instruction_Cache[instr_pc+1]);
        instr_pc+=2;
        pipeline_stage++;
    }

    void instr_decode()
    {
        int temp = IR;
        opcode = (temp >> 12)&15;
        R1 = (temp >> 8)&15;
        R2 = (temp >> 4)&15;
        R3 = (temp)&15;
        //opcode, R1, R2, R3 contains the first 4 , 2nd 4, 3rd 4 and last 4 bits if IR resp.
        switch(opcode)
        {
        case 0:
            //ADD case
            if(if_reg_free[R2])
                src_reg1 = Register_File[R2];
            else
            {
                stall=1;
                return;
            }
            if(if_reg_free[R3])
                src_reg2 = Register_File[R3];
            else
            {
                stall=1;
                return;
            }
            break;
        case 1:
            //SUB case
        {
            if(if_reg_free[R2])
                src_reg1 = Register_File[R2];
            else
            {
                stall=1;
                return;
            }
            if(if_reg_free[R3])
                src_reg2 = Register_File[R3];
            else
            {
                stall=1;
                return;
            }
            if_reg_free[R1] = 0;
            break;
        }
        case 2:
            //MUL case
        {
            if(if_reg_free[R2])
                src_reg1 = Register_File[R2];
            else
            {
                stall=1;
                return;
            }
            if(if_reg_free[R3])
                src_reg2 = Register_File[R3];
            else
            {
                stall=1;
                return;
            }
            if_reg_free[R1] = 0;
            break;
        }
        case 3:
            //INC case
            if (if_reg_free[R1])
            {
                src_reg1 = Register_File[R1];
                src_reg2 = 1;
            }
            else
            {
                stall=1;
                return;
            }
            break;
        case 4:
            //AND case
            if(if_reg_free[R2])
                src_reg1 = Register_File[R2];
            else
            {
                stall=1;
                return;
            }
            if(if_reg_free[R3])
                src_reg2 = Register_File[R3];
            else
            {
                stall=1;
                return;
            }
            if_reg_free[R1] = 0;
            break;

        case 5:
            //OR case
            if(if_reg_free[R2])
                src_reg1 = Register_File[R2];
            else
            {
                stall=1;
                return;
            }
            if(if_reg_free[R3])
                src_reg2 = Register_File[R3];
            else
            {
                stall=1;
                return;
            }
            break;
        case 6:
            //NOT case
            if(if_reg_free[R2])
                src_reg1 = Register_File[R2];
            else
            {
                stall = 1;
                return;
            }
            if_reg_free[R1] = 0;
            break;
        case 7:
            //XOR case
            if(if_reg_free[R2])
                src_reg1 = Register_File[R2];
            else
            {
                stall = 1;
                return;
            }
            if(if_reg_free[R3])
                src_reg2 = Register_File[R3];
            else
            {
                stall = 1;
                return;
            }
            break;
        case 8:
					//LOAD CASE
            if(if_reg_free[R2])
                src_reg1 = Register_File[R2];
            else
            {
                stall = 1;
                return;
            }
                Immediate_val = R3;
            if_reg_free[R1] = 0;
            break;
        case 9:
					//STORE CASE
			if(if_reg_free[R2])
            {
              src_reg1 = Register_File[R2];
        }
        else
            {
                stall = 1;
                return;
            }
        if(if_reg_free[R1])
            src_reg2 = Register_File[R1];
            else
            {
                stall = 1;
                return;
            }
                Immediate_val = R3;
					break;
        case 10:
					//JMP CASE
					src_reg1 = (R1 << 4) + R2;
					src_reg1 = convert_to_32(src_reg1);
          fetch_allowed = 0;
					break;
        case 11:
					//BEQZ CASE
					if(if_reg_free[R1])
                src_reg1 = Register_File[R1];
            else
            {
                stall = 1;
                return;
            }
					src_reg2 = (R2 << 4) + R3;
					src_reg2 = convert_to_32(src_reg2);
          fetch_allowed = 0;
					break;
        case 15:
					halt = 1;
					break;
					//HALT CASE
        }
			stall=0;
    }
    void execute_cycle()
    {
      switch(opcode)
        {
          case 0:
						//ADD CASE
            ALU_Output = src_reg1 + src_reg2;
            break;
          case 1:
						//SUB CASE
            ALU_Output = src_reg1 - src_reg2;
            break;
          case 2:
						//MUL CASE
            ALU_Output = src_reg1 * src_reg2;
            break;
          case 3:
						//INC CASE
            ALU_Output = src_reg1 + src_reg2;
            break;
          case 4:
						//AND
            ALU_Output = src_reg1 & src_reg2;
            break;
          case 5:
						//OR
            ALU_Output = src_reg1 | src_reg2;
            break;
          case 6:
						//NOT
            ALU_Output = ~src_reg1;
            break;
          case 7://XOR
            ALU_Output = src_reg1 ^ src_reg2;
            break;
          case 8://LOAD
            ALU_Output = src_reg1 + Immediate_val;
            break;
          case 9://STORE
            ALU_Output = src_reg1 + Immediate_val;
            break;
          case 10://JMP
            ALU_Output = instr_pc + (src_reg1 << 1);
            break;
          case 11://BEQZ
            ALU_Output = instr_pc + (src_reg2 << 1);
            if(src_reg1 == 0)
              eqz = 1;
            break;
        }
    }

	void memory_cycle()
	{
		//accessing memory from cache and writing it to the memory location in case of store instruction
	    switch (opcode)
	    {
	    case 8:
				//LOAD
				  LMD = Data_Cache[ALU_Output];
	        break;
	    case 9:
				//STORE
          Data_Cache[ALU_Output] = src_reg2;
	        break;
	    case 10:
				//JMP
          PC =  ALU_Output;
          fetch_allowed = 1;
	        break;
	    case 11:
				//BEQZ
          if(eqz)
            PC = ALU_Output;
            fetch_allowed = 1;
	        break;
	    case 15: break;
	    }
	}

  void write_back()
  {
		//writing back to register from memory locations
    switch(opcode)
      {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
			//Converting the ALU_output to 8 bit 2s complement form
			//and assigning it to register value
      ALU_Output = convert_to_8(ALU_Output);
      Register_File[R1] = ALU_Output;
      if_reg_free[R1] = 1;
      break;
    case 8:
        LMD = convert_to_8(LMD);
        Register_File[R1] = LMD;

        if_reg_free[R1] = 1;
    }
  }

  void perform_instr()
    { // Process
        switch (pipeline_stage)
        {
        case 1:
            instr_decode();
            break;
        case 2:
            execute_cycle();
            break;
        case 3:
            memory_cycle();
            break;
        case 4:
            write_back();
            break;
        }
        if (!stall) pipeline_stage++;
        if(pipeline_stage==5)
          calc_stats();
    }
  void calc_stats()
  {
    if (opcode < 4)
      arithmetic_count++;
    else if (opcode < 8)
      logical_count++;
    else if (opcode < 10)
      data_count++;
    else if (opcode < 12)
      control_count++;
    else
      halts_count++;
  }
};

void Instruction_Input()
{
    //reading the instructions from input file to the instruction_cache array
    ifstream inputfile;
    inputfile.open("ICache.txt");
    string instruction;
    for(int i = 0; i < 256 ; i++)
    {
        if(!inputfile) break;
        getline(inputfile, instruction);
        Instruction_Cache[i] = stoi(instruction,0,16);
    }
    inputfile.close();
}

void Data_Input()
{
    //reading the data from input cache file to the data_cache arrray
    ifstream inputfile;
    inputfile.open("DCache.txt");
    string data;
    for(int i = 0; i < 256; i++)
    {
        if(!inputfile) break;
        getline(inputfile, data);
        Data_Cache[i] = stoi(data,0,16);
    }
    inputfile.close();
}

void Register_Input()
{
    //reading the register data from input register RF file to the register input array
    ifstream inputfile;
    inputfile.open("RF.txt");
    string reg;
    for(int i = 0; i < 16; i++)
    {
        if(!inputfile) break;
        getline(inputfile, reg);
        Register_File[i] = stoi(reg,0,16);
    }

    inputfile.close();
}

void PrintOutput()
{
    ofstream outputfile;
    outputfile.open("ODCache.txt");
    for(int i = 0; i < 256; i++)
    {
        int temp = Data_Cache[i];
        temp = temp & 0xff;
        //last 8 digits into temp
        outputfile << hex << (temp >> 4);
        //sending the first 4 digits
        temp = temp & 0xf;
        //last 4 digits into temp
        outputfile << hex << temp << endl;
        //hex form of last 4 digits into outputfile
    }
    outputfile.close();
}

void print_stats(string filename)
{ // Printing Stats
    ofstream output;
    output.open(filename);
    output << "Total number of instructions executed:" << instr_count << endl;
    output << "Number of instructions in each class" << endl;
    output << "Arithmetic instructions              :" << arithmetic_count << endl;
    output << "Logical instructions                 :" << logical_count << endl;
    output << "Data instructions                    :" << data_count << endl;
    output << "Control instructions                 :" << control_count<< endl;
    output << "Halt instructions                    :" << halts_count << endl;
    output << "Cycles Per Instruction               :" << (double)cycles_count / instr_count << endl;
    output << "Total number of stalls               :" << stall_count << endl;
    output << "Data stalls (RAW)                    :" << data_stall_count << endl;
    output << "Control stalls                       :" << control_stall_count << endl;
    output.close();
}

void initialise()
{
    for(int i=0 ; i<16 ; i++)
        if_reg_free[i] = 1;
	//initially all register are free
}

queue<Instr> instr_Q;
//queue for instructions currently running in the processor
int main()
{
    Instruction_Input();//storing in the array
    Data_Input();//storing data (memory) into data array
    Register_Input();//story register values into register array
    initialise();
    do
      {
				//each cycle
        cycles_count++;
        int stall1 = 0;
        int n = instr_Q.size();
        for (int i = 0; i < n; i++)
        {
					//performing one cycle of the each instruction  in the queue
            Instr ins = instr_Q.front();
            instr_Q.pop();
            ins.perform_instr();
            stall1 += ins.ret_stall();
            if (ins.pipeline_stage < 5)
                instr_Q.push(ins);
        }
        if(!fetch_allowed)
        {
          stall_count++;
          control_stall_count++;
        }
        else if (stall1)
        {
            stall_count++;
            data_stall_count++;
        }
        else if (!halt)
        {
            instr_count++;
            Instr next_ins;
            next_ins.fetch_cycle();
            instr_Q.push(next_ins);
        }
      }
			while	(!instr_Q.empty());
    PrintOutput();
    print_stats("Output.txt");
}
