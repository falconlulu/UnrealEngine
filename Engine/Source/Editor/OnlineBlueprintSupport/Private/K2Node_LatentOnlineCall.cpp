// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "OnlineBlueprintSupportPrivatePCH.h"
#include "OnlineBlueprintCallProxyBase.h"
#include "K2Node_LatentOnlineCall.h"
#include "BlueprintFunctionNodeSpawner.h"

#define LOCTEXT_NAMESPACE "K2Node"

UK2Node_LatentOnlineCall::UK2Node_LatentOnlineCall(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UOnlineBlueprintCallProxyBase, Activate);
}

void UK2Node_LatentOnlineCall::GetMenuEntries(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	const bool bAllowLatentFuncs = (K2Schema->GetGraphType(ContextMenuBuilder.CurrentGraph) == GT_Ubergraph);


	if (bAllowLatentFuncs)
	{
		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			UClass* TestClass = *ClassIt;
			if (TestClass->IsChildOf(UOnlineBlueprintCallProxyBase::StaticClass()) && !TestClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				for (TFieldIterator<UFunction> FuncIt(TestClass, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
				{
					// See if the function is a static factory method for online proxies
					UFunction* CandidateFunction = *FuncIt;

					UObjectProperty* ReturnProperty = Cast<UObjectProperty>(CandidateFunction->GetReturnProperty());
					const bool bValidReturnType = (ReturnProperty != nullptr) && (ReturnProperty->PropertyClass != nullptr) && (ReturnProperty->PropertyClass->IsChildOf(UOnlineBlueprintCallProxyBase::StaticClass()));

					if (CandidateFunction->HasAllFunctionFlags(FUNC_Static) && bValidReturnType)
					{
						// Create a node template for this factory method
						UK2Node_LatentOnlineCall* NodeTemplate = NewObject<UK2Node_LatentOnlineCall>(ContextMenuBuilder.OwnerOfTemporaries);
						NodeTemplate->ProxyFactoryFunctionName = CandidateFunction->GetFName();
						NodeTemplate->ProxyFactoryClass = TestClass;
						NodeTemplate->ProxyClass = ReturnProperty->PropertyClass;

						CreateDefaultMenuEntry(NodeTemplate, ContextMenuBuilder);
					}
				}
			}
		}
	}
}

void UK2Node_LatentOnlineCall::GetMenuActions(TArray<UBlueprintNodeSpawner*>& ActionListOut) const
{
	// these nested loops are combing over the same classes/functions the
	// FBlueprintActionDatabase does; ideally we save on perf and fold this in
	// with FBlueprintActionDatabase, but we want to keep the modules separate
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		if (!Class->IsChildOf<UOnlineBlueprintCallProxyBase>() || Class->HasAnyClassFlags(CLASS_Abstract))
		{
			continue;
		}
		
		for (TFieldIterator<UFunction> FuncIt(Class, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
		{
			UFunction* Function = *FuncIt;
			if (!Function->HasAnyFunctionFlags(FUNC_Static))
			{
				continue;
			}
			
			UObjectProperty* ReturnProperty = Cast<UObjectProperty>(Function->GetReturnProperty());
			// see if the function is a static factory method for online proxies
			bool const bIsProxyFactoryMethod = (ReturnProperty != nullptr) && ReturnProperty->PropertyClass->IsChildOf<UOnlineBlueprintCallProxyBase>();
			
			if (bIsProxyFactoryMethod)
			{
				UBlueprintNodeSpawner* NodeSpawner = UBlueprintFunctionNodeSpawner::Create(Function);
				check(NodeSpawner != nullptr);
				NodeSpawner->NodeClass = GetClass();
				
				auto CustomizeAcyncNodeLambda = [](UEdGraphNode* NewNode, bool bIsTemplateNode, TWeakObjectPtr<UFunction> FunctionPtr)
				{
					UK2Node_LatentOnlineCall* AsyncTaskNode = CastChecked<UK2Node_LatentOnlineCall>(NewNode);
					if (FunctionPtr.IsValid())
					{
						UFunction* Function = FunctionPtr.Get();
						UObjectProperty* ReturnProperty = CastChecked<UObjectProperty>(Function->GetReturnProperty());
						
						AsyncTaskNode->ProxyFactoryFunctionName = Function->GetFName();
						AsyncTaskNode->ProxyFactoryClass        = Function->GetOuterUClass();
						AsyncTaskNode->ProxyClass               = ReturnProperty->PropertyClass;
					}
				};
				
				TWeakObjectPtr<UFunction> FunctionPtr = Function;
				NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeAcyncNodeLambda, FunctionPtr);
				
				// @TODO: since this can't be folded into FBlueprintActionDatabase, we
				//        need a way to associate these spawners with a certain class
				ActionListOut.Add(NodeSpawner);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
