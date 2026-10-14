// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UGameProjectSummer26AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGameProjectSummer26AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGameProjectSummer26AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGameProjectSummer26AttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGameProjectSummer26AttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGameProjectSummer26AttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGameProjectSummer26AttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGameProjectSummer26AttributeSet, Endurance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGameProjectSummer26AttributeSet, MaxEndurance, COND_None, REPNOTIFY_Always);
}

void UGameProjectSummer26AttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UGameProjectSummer26AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UGameProjectSummer26AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	ClampCurrentAttributes();
}

void UGameProjectSummer26AttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetMaxHealthAttribute() || Attribute == GetMaxStaminaAttribute() ||
		Attribute == GetMaxManaAttribute() || Attribute == GetMaxEnduranceAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetEnduranceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxEndurance());
	}
}

void UGameProjectSummer26AttributeSet::ClampCurrentAttributes()
{
	SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	SetEndurance(FMath::Clamp(GetEndurance(), 0.0f, GetMaxEndurance()));
}

void UGameProjectSummer26AttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGameProjectSummer26AttributeSet, Health, OldValue);
}

void UGameProjectSummer26AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGameProjectSummer26AttributeSet, MaxHealth, OldValue);
}

void UGameProjectSummer26AttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGameProjectSummer26AttributeSet, Stamina, OldValue);
}

void UGameProjectSummer26AttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGameProjectSummer26AttributeSet, MaxStamina, OldValue);
}

void UGameProjectSummer26AttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGameProjectSummer26AttributeSet, Mana, OldValue);
}

void UGameProjectSummer26AttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGameProjectSummer26AttributeSet, MaxMana, OldValue);
}

void UGameProjectSummer26AttributeSet::OnRep_Endurance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGameProjectSummer26AttributeSet, Endurance, OldValue);
}

void UGameProjectSummer26AttributeSet::OnRep_MaxEndurance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGameProjectSummer26AttributeSet, MaxEndurance, OldValue);
}
