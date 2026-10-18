#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RsTargeting.h"
#include "RsEnemy.generated.h"

class UWidgetComponent;
class USphereComponent;
class URsStateComponent;
class URsAttributeComponent;

// 武器のPointDamageを受け、体力・命中演出・方向別の被弾反応・死亡時の物理挙動を管理する敵。
UCLASS()
class RYUSOULS_API ARsEnemy : public ACharacter, public IRsTargeting
{
	GENERATED_BODY()

protected:
	// 専用Object Typeで候補検索に参加し、Visibilityの視線トレースも受ける球。
	UPROPERTY(VisibleAnywhere)
	USphereComponent* TargetingSphereComponent;

	// 体力の減算と死亡通知を受け持つ。OnDeathへ敵の死亡処理を接続する。
	UPROPERTY(VisibleAnywhere)
	URsAttributeComponent* AttributeComponent;

	// 体力が0になったときのDeathタグを保持する。
	UPROPERTY(VisibleAnywhere)
	URsStateComponent* StateComponent;

	// ロック対象になったときに表示する画面空間のWidget。表示クラスは敵BPで指定する。
	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* LockOnWidgetComponent;

protected:
	// PointDamageを受けた位置で再生する効果音。敵BPで割り当てる。
	UPROPERTY(EditAnywhere, Category="Effect")
	USoundCue* ImpactSound;

	// PointDamageのImpactPointに生成する命中パーティクル。
	UPROPERTY(EditAnywhere, Category="Effect")
	UParticleSystem* ImpactParticle;

protected:
	// 敵の正面から攻撃された場合に使う被弾Montage。
	UPROPERTY(EditAnywhere, Category = "Montage | HitReact")
	UAnimMontage* HitReactAnimFront;

	// 敵の背面から攻撃された場合に使う被弾Montage。
	UPROPERTY(EditAnywhere, Category = "Montage | HitReact")
	UAnimMontage* HitReactAnimBack;

	// 敵の左側から攻撃された場合に使う被弾Montage。
	UPROPERTY(EditAnywhere, Category = "Montage | HitReact")
	UAnimMontage* HitReactAnimLeft;

	// 敵の右側から攻撃された場合に使う被弾Montage。
	UPROPERTY(EditAnywhere, Category = "Montage | HitReact")
	UAnimMontage* HitReactAnimRight;

public:
	ARsEnemy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	// 基底クラスが返す実ダメージで体力を更新し、PointDamageなら命中演出と被弾反応も行う。
	virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:
	// カプセルの衝突を止め、メッシュをRagdollの物理シミュレーションへ切り替える。
	virtual void OnDeath();

protected:
	// 指定されたワールド座標で、BPに設定した命中音とパーティクルを再生する。
	void ImpactEffect(const FVector& Location);
	// 攻撃者との位置関係に対応する被弾Montageを選んで再生する。
	void HitReaction(const AActor* Attacker);
	// 自身の向きと攻撃者へ向くYawの差を4方向へ分類し、対応するMontageを返す。
	UAnimMontage* GetHitReactAnimation(const AActor* Attacker) const;

public:
	// IRsTargetingからの対象変更通知を受け、頭上のロックWidgetを表示・非表示にする。
	virtual void OnTargeted(bool bTargeted) override;
	// 状態コンポーネントがあり、Death状態ではないときだけ対象にできる。
	virtual bool CanBeTargeted() override;
};
