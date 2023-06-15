//@BridgetACasey - 1802644

#include "BlendNodeBase.h"
#include "BlendTree.h"

namespace AnimationLib
{
    BlendNodeBase::BlendNodeBase(BlendTree* tree)
    {
        blendTree = tree;
    }

    BlendNodeBase::~BlendNodeBase()
    {
    }

    bool BlendNodeBase::UpdateNode(float frameTime)
    {
        bool allInputsValid = true;

        for(int inputNum = 0; inputNum < inputs.size(); inputNum++)
        {
            bool inputIsValid = false;

            //Only update the node if it matches known inputs - accessing by depth first
            if (inputs[inputNum])
                inputIsValid = inputs[inputNum]->UpdateNode(frameTime);

            if (!inputIsValid)
                allInputsValid = false;
        }


        bool allVariablesAreValid = true;

        if (!variables.empty())
        {
            for (auto& name : variables)
            {
                //check the variable exists
                
                const bool validVariable = (blendTree->GetTreeVariables().find(name) != blendTree->GetTreeVariables().end());

                if (!validVariable && allVariablesAreValid)
                {
                    allVariablesAreValid = false;
                }
            }
        }

        bool outputValid = false;

        if (allInputsValid)
            outputValid = ProcessNode(frameTime);

        return outputValid;
    }

    void BlendNodeBase::SetInput(BlendNodeBase* inputNode, int inputIndex)
    {
    	inputs.at(inputIndex) = inputNode;
    }

    void BlendNodeBase::SetVariable(std::string variable, int inputIndex)
    {
        variables.at(inputIndex) = variable;
    }
}
