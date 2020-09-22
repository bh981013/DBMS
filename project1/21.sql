select t.name, count(c.pid) as 'num'
from CatchedPokemon c, Trainer t
where c.owner_id = t.id and
      c.owner_id in ( select r.id
       from Gym g, Trainer r
       where g.leader_id = r.id
      )
group by t.name
order by t.name;