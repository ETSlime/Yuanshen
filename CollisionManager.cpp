//=============================================================================
//
// 当たり判定処理 [CollisionManager.cpp]
// Author : 
//
//=============================================================================
#include "CollisionManager.h"
#include "GameObject.h"

constexpr float HEIGHT_EPSILON = 1.0f;
constexpr float SMOOTHING_FACTOR = 0.2f;
// 最大許容斜面角度（ラジアン、例：45度）
const float MAX_SLOPE_ANGLE = 0.785398f; // 約45度
// 階段として許容する最大高さ差（ステップオフセット）
const float STEP_OFFSET = 5.0f;

void CollisionManager::InitOctree(const BOUNDING_BOX& boundingBox)
{
    if (staticOctree == nullptr)
        staticOctree = new OctreeNode(boundingBox);
    else
    {
        delete staticOctree;
        staticOctree = new OctreeNode(boundingBox);
    }
}

void CollisionManager::Update()
{
    int numColliders = dynamicColliders.getSize();

    // 動的オブジェクトと静的オブジェクト（地面）の衝突検出
    if (staticOctree)
    {
        for (int i = 0; i < numColliders; i++)
        {
            const Collider* dynamicCol = dynamicColliders[i];
            if (dynamicCol->type == ColliderType::WALL) continue;

            auto colliderOwner = static_cast<GameObject<SkinnedMeshModelInstance>*>(dynamicCol->owner);
            Transform transform = colliderOwner->GetTransform();


            // 動的オブジェクトの AABB を元に、静的八分木から候補となる三角形をクエリする
            SimpleArray<Triangle*> candidates;
            staticOctree->queryRange(dynamicCol->aabb, candidates);

            bool hasCollision = false;
            bool hasValidSlope = false;
            float bestY = -FLT_MAX;
            float minHeightDiff = FLT_MAX;
            XMVECTOR obstacleNormal{};
            // 候補となる各三角形に対して、精密な衝突判定を実施する
            int numCandidates = candidates.getSize();
            for (int j = 0; j < numCandidates; j++)
            {
                SimpleArray<float> hitYArray;
                if (dynamicCol->aabb.intersects(candidates[j]->bbox))
                {
                    hasCollision = true;
                    // 動的オブジェクトがプレイヤーと敵の場合のみ、斜面の高さ補正を計算する
                    if (dynamicCol->type == ColliderType::PLAYER)
                    {
                        XMFLOAT3 n = candidates[j]->normal;

                        // 障害物の法線 n
                        obstacleNormal = XMVectorSet(n.x, 0, n.z, 0);
                        obstacleNormal = XMVector3Normalize(obstacleNormal);

                        // n.y が 0 に近い場合は、斜面判定を無視
                        if (fabs(n.y) < 0.001f)
                            continue;

                        // 斜面角度を計算する（acos(n.y)；n は正規化済み）
                        float slopeAngle = acosf(n.y);

                        float posX = transform.pos.x;
                        float posZ = transform.pos.z;

                        // 候補となる三角形の基準点（v0）の高さを取得する
                        float baseY = candidates[j]->v0.y;

                        // 候補三角形のv0との水平方向の差を計算する
                        float deltaX = posX - candidates[j]->v0.x;
                        float deltaZ = posZ - candidates[j]->v0.z;

                        // 法線 n の x, z 成分に水平方向の差をかけ、合計する
                        float horizontalComponent = n.x * deltaX + n.z * deltaZ;

                        // colliderが平面上でどの高さに来るかを計算する
                        float heightAdjustment = horizontalComponent / n.y;

                        // 衝突面上のcolliderの y 座標（hitY）を求める
                        float candidateHitY = baseY - horizontalComponent / n.y;

                        float heightDiff = fabs(candidateHitY - transform.pos.y);
                        if (slopeAngle <= MAX_SLOPE_ANGLE || heightDiff < STEP_OFFSET)
                        {
                            // 斜面が許容範囲内の場合
                            minHeightDiff = heightDiff;
                            bestY = candidateHitY;
                            hasValidSlope = true;
                        }
                    }
                    
                    CollisionEvent collisionEvent;
                    collisionEvent.colliderA = dynamicCol;
                    // 静的オブジェクトは Triangle* で管理しているので、ここでは衝突対象の種類を WALL とする
                    static Collider dummyStatic;
                    dummyStatic.aabb = candidates[j]->bbox;
                    dummyStatic.type = ColliderType::WALL;
                    collisionEvent.colliderB = &dummyStatic;
                    eventBus.publishCollision(collisionEvent);
                }
            }


            if (dynamicColliders[i]->type == ColliderType::PLAYER)
            {
                if (hasValidSlope)
                {
                    if (fabs(transform.pos.y - bestY) > HEIGHT_EPSILON)
                    {
                        // 現在のcolliderの位置を XMVECTOR に変換
                        XMVECTOR currentPos = XMLoadFloat3(&transform.pos);

                        // 目標のcolliderの位置は、x, z はそのままで y を hitY に設定する
                        XMVECTOR targetPos = XMVectorSet(transform.pos.x, bestY, transform.pos.z, 0.0f);

                        // XMVectorLerp を用いて、現在位置と目標位置の間を補間する
                        XMVECTOR lerpedPos = XMVectorLerp(currentPos, targetPos, SMOOTHING_FACTOR);

                        // 補間結果をcolliderの位置に戻す
                        XMStoreFloat3(&transform.pos, lerpedPos);

                        colliderOwner->SetTransform(transform);
                    }
                }
                else if (hasCollision)
                {
                    // 高い障害物の場合：移動ベクトルの調整（滑るように動く）
                    
                    Attributes attributes = colliderOwner->GetAttributes();

                    // 意図する移動ベクトルの計算
                    float intendedMoveX = sinf(transform.rot.y) * attributes.spd;
                    float intendedMoveZ = cosf(transform.rot.y) * attributes.spd;

                    // 意図する移動ベクトルを DirectX の XMVECTOR に変換（Y成分は 0）
                    XMVECTOR intendedMove = XMVectorSet(intendedMoveX, 0, intendedMoveZ, 0);
                    intendedMove = XMVector3Normalize(intendedMove);  // 正規化

                    // 障害物との衝突がある場合、障害物法線（水平成分のみ）との内積を計算
                    float dot = XMVectorGetX(XMVector3Dot(intendedMove, obstacleNormal));

                    // dot > 0 なら、障害物方向に移動しようとしているので、その成分を除去
                    if (dot > 0.0f)
                    {
                        XMVECTOR projection = XMVectorScale(obstacleNormal, dot);
                        XMVECTOR allowedMove = XMVectorSubtract(intendedMove, projection);
                        allowedMove = XMVector3Normalize(allowedMove);

                        // 修正後の移動ベクトルに、元の移動スカラー（速度）をかける
                        float spd = attributes.spd; // 元の移動スカラー
                        XMVECTOR finalMove = XMVectorScale(allowedMove, spd);

                        // 最終的な移動量を取得して、transform.pos に加算する
                        XMFLOAT3 moveDelta;
                        XMStoreFloat3(&moveDelta, finalMove);
                        transform.pos.x += moveDelta.x;
                        transform.pos.z += moveDelta.z;
                    }
                    else
                    {
                        // dot が 0 以下の場合は、障害物方向には移動していないので、そのまま加算する
                        transform.pos.x += intendedMoveX;
                        transform.pos.z += intendedMoveZ;
                    }

                    colliderOwner->SetTransform(transform);
                }
                bool isGrounded = colliderOwner->GetAttributes().isGrounded;
                // 着地状態（isGrounded）の更新を行う
                if (hasCollision && !isGrounded)
                {
                    colliderOwner->SetGrounded(true);
                }
                else if (!hasCollision && isGrounded)
                {
                    colliderOwner->SetGrounded(false);
                }
            }
            
        }


        // 動的オブジェクト同士（例：プレイヤーと敵、その他）の衝突検出
        for (int i = 0; i < numColliders; i++)
        {
            for (int j = i + 1; j < numColliders; j++)
            {
                if (dynamicColliders[i]->aabb.intersects(dynamicColliders[j]->aabb))
                {
                    CollisionEvent collisionEvent;
                    collisionEvent.colliderA = dynamicColliders[i];
                    collisionEvent.colliderB = dynamicColliders[j];
                    eventBus.publishCollision(collisionEvent);
                }
            }
        }
    }
}

//bool CollisionManager::PreciseCollision(Collider* col, Triangle* tri)
//{
//	return false;
//}
//
//bool CollisionManager::PreciseCollision(Collider* col1, Collider* col2)
//{
//	return false;
//}
