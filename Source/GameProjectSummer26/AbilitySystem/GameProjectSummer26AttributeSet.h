// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameProjectSummer26AttributeSet.generated.h"

#define GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** Core resource attributes shared by player abilities. */
UCLASS()
class GAMEPROJECTSUMMER26_API UGameProjectSummer26AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes|Health")
	FGameplayAttributeData Health;
	GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(UGameProjectSummer26AttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes|Health")
	FGameplayAttributeData MaxHealth;
	GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(UGameProjectSummer26AttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Attributes|Stamina")
	FGameplayAttributeData Stamina;
	GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(UGameProjectSummer26AttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Attributes|Stamina")
	FGameplayAttributeData MaxStamina;
	GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(UGameProjectSummer26AttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Attributes|Mana")
	FGameplayAttributeData Mana;
	GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(UGameProjectSummer26AttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Attributes|Mana")
	FGameplayAttributeData MaxMana;
	GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(UGameProjectSummer26AttributeSet, MaxMana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Endurance, Category = "Attributes|Endurance")
	FGameplayAttributeData Endurance;
	GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(UGameProjectSummer26AttributeSet, Endurance)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxEndurance, Category = "Attributes|Endurance")
	FGameplayAttributeData MaxEndurance;
	GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS(UGameProjectSummer26AttributeSet, MaxEndurance)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Endurance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxEndurance(const FGameplayAttributeData& OldValue);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	void ClampCurrentAttributes();
};

#undef GAMEPROJECTSUMMER26_ATTRIBUTE_ACCESSORS
