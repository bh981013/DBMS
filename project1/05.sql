select r.name
from Trainer r
where r.name not in (select t.name
from Trainer t, Gym g
where t.id = g.leader_id)
order by r.name;